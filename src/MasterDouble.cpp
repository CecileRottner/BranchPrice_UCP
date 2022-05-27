#include "Master.h"

/* namespace usage */
using namespace std;
using namespace scip;





void MasterDouble_Model::addCoefsToConstraints_siteVar(SCIP* scip, Master_Variable* lambda, InstanceUCP* inst) {

    int s = lambda->Site ;
    int first = Param.firstUnit(s) ;

    /* add coefficient to the convexity constraint for site s */
    SCIPaddCoefLinear(scip, conv_lambda_site.at(s), lambda->ptr, 1.0) ;

//    /* add coef to the equality x(site) = x(time)*/
    for (int t=0 ; t < T ; t++) {
        for (int i=0 ; i < Param.nbUnits(s) ; i++) {

            if (!Param.powerPlanGivenByLambda) {
                if (lambda->UpDown_plan[i*T+t] > 1 - Param.Epsilon) {
                    SCIPaddCoefLinear(scip, eq_time_site.at((first+i)*T+t), lambda->ptr, 1.0) ;
                    if (Param.PminDifferentPmax && !Param.powerPlanGivenByMu){
                        SCIPaddCoefLinear(scip, power_limits[(first+i)*T+t], lambda->ptr, inst->getPmax(first+i) - inst->getPmin(first+i)) ;
                        if (lambda->UpDown_plan[i*T+t] > 1 - Param.Epsilon) {
                            SCIPaddCoefLinear(scip, demand_cstr[t], lambda->ptr, inst->getPmin(first+i)) ;
                        }
                    }
                }
            }

            else {
            }
        }
    }

    if (Param.minUpDownDouble && !Param.useUVar) {

        //Logical constraint coefs
        for (int i=0 ; i < Param.nbUnits(s) ; i++) {
            for (int t=1 ; t < T ; t++) {

                if ( (lambda->UpDown_plan[i*T+t] > 1 - Param.Epsilon)  && (lambda->UpDown_plan[i*T+t-1] < Param.Epsilon) ) {
                    SCIPaddCoefLinear(scip, logical.at((first+i)*T+t), lambda->ptr, -1.0) ;
                }
            }
        }
        /* coef in min-up / min-down constraints */
        for (int i=0 ; i < Param.nbUnits(s) ; i++) {

            int L = inst->getL(first+i) ;
            int l = inst->getl(first+i) ;
            //min up
            for (int t=L ; t < T ; t++) {
                for (int k=t-L+1 ; k <= t ; k++) {


                    if ( (lambda->UpDown_plan[i*T+k] > 1 - Param.Epsilon)  && (lambda->UpDown_plan[i*T+k-1] < Param.Epsilon) ) {
                        SCIPaddCoefLinear(scip, min_up.at((first+i)*T+t), lambda->ptr, -1.0) ;
                    }
                }
            }

            //min-down
            for (int t=l ; t < T ; t++) {
                for (int k=t-l+1 ; k <= t ; k++) {
                    if ( (lambda->UpDown_plan[i*T+k] > 1 - Param.Epsilon)  && (lambda->UpDown_plan[i*T+k-1] < Param.Epsilon) ) {
                        SCIPaddCoefLinear(scip, min_down.at((first+i)*T+t), lambda->ptr, 1.0) ;
                    }
                }
            }

        }
    }

}

void MasterDouble_Model::addCoefsToConstraints_timeVar(SCIP* scip, MasterTime_Variable* lambda) {

    int t = lambda->time ;

    /* add coefficient to the convexity constraint for site s */
    SCIPaddCoefLinear(scip, conv_lambda_time.at(t), lambda->ptr, 1.0) ;

    /* add coef to the equality x(site) = x(time)*/

    for (int i=0 ; i < n ; i++) {
        if (lambda->UpDown_plan[i] > 1 - Param.Epsilon) {
            SCIPaddCoefLinear(scip, eq_time_site.at(i*T+t), lambda->ptr, -1.0) ;
        }
    }

    if (Param.minUpDownDouble) {
        /* for each unit i, add coefficient 1 to logical constraint logical(i,t) if t >=1, add coefficient -1 to logical(i, t+1) if t < T-1*/

        for (int i=0 ; i < n ; i++) {
            if (lambda->UpDown_plan[i] > 1 - Param.Epsilon) {
                if (t>0) {
                    SCIPaddCoefLinear(scip, logical.at(i*T+t), lambda->ptr, 1.0) ;
                }
                if (t < T-1) {
                    SCIPaddCoefLinear(scip, logical.at(i*T+t+1), lambda->ptr, -1.0) ;
                }
            }
        }

        /* coef in min-up / min-down constraints */
        for (int i=0 ; i < n ; i++) {
            if (lambda->UpDown_plan[i] > 1 - Param.Epsilon) {
                if (t>=inst->getL(i)) {
                    SCIPaddCoefLinear(scip, min_up.at(i*T+t), lambda->ptr, 1.0) ;
                }
                int l = inst->getl(i) ;
                if (t < T - l) {
                    SCIPaddCoefLinear(scip, min_down.at(i*T+t+l), lambda->ptr, 1.0) ;
                }
            }
        }
    }
}

void MasterDouble_Model::initMasterSiteVariable(SCIP* scip, InstanceUCP* inst , Master_Variable* var) {
    char var_name[255];
    SCIPsnprintf(var_name, 255, "V_%d",L_var_site.size());
    SCIPdebugMsg(scip, "new variable <%s>\n", var_name);

    /* create the new variable: Use upper bound of infinity such that we do not have to care about
     * the reduced costs of the variable in the pricing. The upper bound of 1 is implicitly satisfied
     * due to the set partitioning constraints.
     */

    var->computeCost(inst, Param);
    double cost= var->cost;

    //cout << "cost: " << cost << endl ;

    SCIP_Vartype type ;
    if (Param.IP) {
        type = SCIP_VARTYPE_INTEGER;
    }
    else {
        type = SCIP_VARTYPE_CONTINUOUS ;
    }
    SCIPcreateVar(scip, &(var->ptr), var_name,
                  0.0,                     // lower bound
                  SCIPinfinity(scip),      // upper bound
                  cost,                     // objective
                  type,    // variable type
                  false, false, NULL, NULL, NULL, NULL, NULL);
    //// Add new variable to the list
    L_var_site.push_back(var);


    int print =1;
    if (print) {
        cout << "Variable " << var_name << " added, with plan: " << endl  ;
        int first = Param.firstUnit(var->Site) ;
        int last = Param.firstUnit(var->Site) + Param.nbUnits(var->Site) - 1 ;
        for (int i = first ; i <= last ; i++) {
            for (int t=0 ; t < inst->getT() ; t++) {
                cout << var->UpDown_plan[(i-first)*T + t] << " "  ;
            }
            cout << endl;
        }
        cout << "and cost : " << endl;
        cout << cost << endl;
    }
}

//////// Initialisation d'une variable lambda(time) /////////////
void MasterDouble_Model::initMasterTimeVariable(SCIP* scip, MasterTime_Variable* var) {

    char var_name[255];
    SCIPsnprintf(var_name, 255, "W_%d",L_var_time.size());
    SCIPdebugMsg(scip, "new variable <%s>\n", var_name);

    /* create the new variable: Use upper bound of infinity such that we do not have to care about
     * the reduced costs of the variable in the pricing. The upper bound of 1 is implicitly satisfied
     * due to the set partitioning constraints.
     */

    var->computeCost(inst, Param);
    double cost= var->cost;
    //cout << var_name << ", cost: " << cost << endl ;

    SCIP_Vartype type ;
    if (Param.IP) {
        type = SCIP_VARTYPE_INTEGER;
    }
    else {
        type = SCIP_VARTYPE_CONTINUOUS ;
    }
    SCIPcreateVar(scip, &(var->ptr), var_name,
                  0.0,                     // lower bound
                  SCIPinfinity(scip),      // upper bound
                  cost,                     // objective
                  type,    // variable type
                  false, false, NULL, NULL, NULL, NULL, NULL);

    //// Add new variable to the list
    L_var_time.push_back(var);

    int print =1;
    if (print) {
        cout << "Variable " << var_name << " added, with plan: " << endl  ;

        for (int i=0 ; i < inst->getn() ; i++) {
            cout << var->UpDown_plan[i] << " "  ;
        }
        cout << endl;
        if (Param.powerPlanGivenByMu){
            cout << "and power plan: " << endl;
            for (int i=0 ; i < inst->getn() ; i++) {
                cout << var->Power_plan[i] << " "  ;
            }
            cout << endl;
        }
        cout << "and cost: " << endl;
        cout << cost << endl;
    }
}

MasterDouble_Model::MasterDouble_Model(InstanceUCP* inst, const Parameters & Parametres) : Master_Model(Parametres, inst) {
    n = inst->getn() ;
    T = inst->getT() ;


    S = Parametres.nbDecGpes ;

    conv_lambda_site.resize(S, (SCIP_CONS*) NULL) ;
    conv_lambda_time.resize(T, (SCIP_CONS*) NULL) ;
    eq_time_site.resize(n*T, (SCIP_CONS*) NULL) ;

    logical.resize(n*T, (SCIP_CONS*) NULL) ;
    min_up.resize(n*T, (SCIP_CONS*) NULL) ;
    min_down.resize(n*T, (SCIP_CONS*) NULL) ;

    power_limits.resize(n*T, (SCIP_CONS*) NULL) ;
    demand_cstr.resize(T, (SCIP_CONS*) NULL);
}

void  MasterDouble_Model::initScipMasterDoubleModel(SCIP* scip, InstanceUCP* inst) {


    ////////////////////////////////////////////////////////////////
    /////////////   MASTER CONSTRAINT INITIALIZATION   /////////////
    ////////////////////////////////////////////////////////////////
    // Constraints form: lhs <= ax <= rhs


    ///// Equality time/site constraint /////
    char con_name_eq_time_site[255];
    SCIP_Real upper_bound =  SCIPinfinity(scip);
    if (!Param.unitGEQTime) {
        upper_bound=0;
    }
    for (int i = 0 ; i <n ; i++)
    {
        for (int t = 0; t < T; t++)
        {
            SCIP_CONS* con = NULL;
            (void) SCIPsnprintf(con_name_eq_time_site, 255, "EqTimeSite(%d,%d)", i, t); // nom de la contrainte
            SCIPcreateConsLinear( scip, &con, con_name_eq_time_site, 0, NULL, NULL,
                                  0.0,   // lhs
                                  upper_bound,   // rhs  SCIPinfinity(scip) if >=1
                                  true,  /* initial */
                                  false, /* separate */
                                  true,  /* enforce */
                                  true,  /* check */
                                  true,  /* propagate */
                                  false, /* local */
                                  true,  /* modifiable */
                                  false, /* dynamic */
                                  false, /* removable */
                                  false  /* stickingatnode */ );
            SCIPaddCons(scip, con);
            eq_time_site.at(i*T + t) = con;
        }
    }



    cout << "convex site cons" << endl ;

    ///// Convexity constraint : site////
    char con_name_convex_site[255];
    for (int s = 0 ; s<S ; s++)
    {
        SCIP_CONS* con = NULL;
        (void) SCIPsnprintf(con_name_convex_site, 255, "ConvSite(%d)", s); // nom de la contrainte
        SCIPcreateConsLinear( scip, &con, con_name_convex_site, 0, NULL, NULL,
                              1.0,   // lhs
                              1.0,   // rhs  SCIPinfinity(scip) if >=1
                              true,  /* initial */
                              false, /* separate */
                              true,  /* enforce */
                              true,  /* check */
                              true,  /* propagate */
                              false, /* local */
                              true,  /* modifiable */
                              false, /* dynamic */
                              false, /* removable */
                              false  /* stickingatnode */ );
        SCIPaddCons(scip, con);
        conv_lambda_site.at(s) = con;
    }



    ///// Convexity constraint : time ////
    char con_name_convex_time[255];
    for (int t = 0 ; t<T ; t++)
    {
        SCIP_CONS* con = NULL;
        (void) SCIPsnprintf(con_name_convex_time, 255, "ConvTime(%d)", t); // nom de la contrainte
        SCIPcreateConsLinear( scip, &con, con_name_convex_time, 0, NULL, NULL,
                              1.0,   // lhs
                              1.0,   // rhs  SCIPinfinity(scip) if >=1
                              true,  /* initial */
                              false, /* separate */
                              true,  /* enforce */
                              true,  /* check */
                              true,  /* propagate */
                              false, /* local */
                              true,  /* modifiable */
                              false, /* dynamic */
                              false, /* removable */
                              false  /* stickingatnode */ );
        SCIPaddCons(scip, con);
        conv_lambda_time.at(t) = con;
    }

    if (Param.PminDifferentPmax && !Param.powerPlanGivenByMu){
        ///// Power limits /////
        char con_name_power_limit[255];
        for (int i = 0 ; i <n ; i++)
        {
            for (int t = 0; t < T; t++)
            {
                SCIP_CONS* con = NULL;
                (void) SCIPsnprintf(con_name_power_limit, 255, "PowerLimit(%d,%d)", i, t); // nom de la contrainte
                SCIPcreateConsLinear( scip, &con, con_name_power_limit, 0, NULL, NULL,
                                    0.0,   // lhs
                                    0.0,   // rhs  SCIPinfinity(scip) if >=1
                                    true,  /* initial */
                                    false, /* separate */
                                    true,  /* enforce */
                                    true,  /* check */
                                    true,  /* propagate */
                                    false, /* local */
                                    true,  /* modifiable */
                                    false, /* dynamic */
                                    false, /* removable */
                                    false  /* stickingatnode */ );
                SCIPaddCons(scip, con);
                power_limits[i*T + t] = con;
            }
        }

        ///// Demand constraint /////
        char con_name_demand[255];
        for (int t = 0; t < T; t++)
        {
            SCIP_CONS* con = NULL;
            (void) SCIPsnprintf(con_name_demand, 255, "Demand(%d)", t); // nom de la contrainte
            SCIPcreateConsLinear( scip, &con, con_name_demand, 0, NULL, NULL,
                                inst->getD(t),   // lhs
                                inst->getD(t),   // rhs  SCIPinfinity(scip) if >=1
                                true,  /* initial */
                                false, /* separate */
                                true,  /* enforce */
                                true,  /* check */
                                true,  /* propagate */
                                false, /* local */
                                true,  /* modifiable */
                                false, /* dynamic */
                                false, /* removable */
                                false  /* stickingatnode */ );
            SCIPaddCons(scip, con);
            demand_cstr[t] = con;
        }

        /////////////////////////////////////////////////////////////
        ////////   MASTER POWER VARIABLES INITIALIZATION   //////////
        /////////////////////////////////////////////////////////////

        cout << "power var init" << endl ;
        char var_name[255];

        for (int i = 0 ; i <n ; i++)
        {
            for (int t = 0; t < T; t++)
            {
                SCIP_VAR* var = NULL;

                SCIPsnprintf(var_name, 255, "p(%d,%d)",i,t);
                SCIPdebugMsg(scip, "Variable <%s>\n", var_name);


                SCIPcreateVar(scip, &var, var_name,
                            0.0,                     // lower bound
                            inst->getPmax(i) - inst->getPmin(i),      // upper bound
                            inst->getcp(i),                     // objective
                            SCIP_VARTYPE_CONTINUOUS, // variable type
                            true, false, NULL, NULL, NULL, NULL, NULL);


                /* add new variable to scip */
                if (!Param.powerPlanGivenByLambda) {
                    SCIPaddVar(scip, var);

                    /* add coefficient to the demand constraint */
                    //SCIPaddCoefLinear(scip, demand_cstr[t], var, 1.0);

                    /* add coefficient to the power limit constraint */
                    SCIPaddCoefLinear(scip, power_limits[i*T + t], var, -1.0);
                }
            }
        }

        cout << "fin" << endl ;


        ///////////////////////////////////////
        ////////   SLACK VARIABLES   //////////
        ///////////////////////////////////////

        char slack_power_name[255];

        for (int i = 0 ; i <n ; i++)
        {
            for (int t = 0; t < T; t++)
            {
                SCIP_VAR* var = NULL;

                SCIPsnprintf(slack_power_name, 255, "slack_power(%d,%d)",i,t);
                SCIPdebugMsg(scip, "Variable <%s>\n", slack_power_name);


                SCIPcreateVar(scip, &var, slack_power_name,
                            0.0,                     // lower bound
                            inst->getPmax(i) - inst->getPmin(i),      // upper bound
                            0.0,                     // objective
                            SCIP_VARTYPE_CONTINUOUS, // variable type
                            true, false, NULL, NULL, NULL, NULL, NULL);


                /* add new variable to scip */
                SCIPaddVar(scip, var);

                /* add coefficient to the power limit constraint */
                SCIPaddCoefLinear(scip, power_limits[i*T + t], var, -1.0);
            }
        }

        char slack_demand_name[255];

        for (int t = 0; t < T; t++)
        {
            SCIP_VAR* var = NULL;

            SCIPsnprintf(slack_demand_name, 255, "slack_demand(%d)",t);
            SCIPdebugMsg(scip, "Variable <%s>\n", slack_demand_name);


            SCIPcreateVar(scip, &var, slack_demand_name,
                        0.0,                     // lower bound
                        10000,      // upper bound
                        0.0,                     // objective
                        SCIP_VARTYPE_CONTINUOUS, // variable type
                        true, false, NULL, NULL, NULL, NULL, NULL);


            /* add new variable to scip */
            SCIPaddVar(scip, var);

            /* add coefficient to the demand constraint */
            SCIPaddCoefLinear(scip, demand_cstr[t], var, -1.0);
        }
    }

    ///////////////// MIN-UP MIN-DOWN constraints for STABILIZATION ////////////////
    ///// LOGICAL ////

    if (Param.minUpDownDouble) {

        ///// Logical constraint /////
        char con_name_logical[255];
        for (int t = 1; t < T; t++)
        {
            for (int i=0 ; i < n ; i++) {
                SCIP_CONS* con = NULL;
                (void) SCIPsnprintf(con_name_logical, 255, "Logical(%d,%d)", i,t); // nom de la contrainte
                SCIPcreateConsLinear( scip, &con, con_name_logical, 0, NULL, NULL,
                                      -SCIPinfinity(scip),   // lhs
                                      0,   // rhs  SCIPinfinity(scip) if >=1
                                      true,  /* initial */
                                      false, /* separate */
                                      true,  /* enforce */
                                      true,  /* check */
                                      true,  /* propagate */
                                      false, /* local */
                                      true,  /* modifiable */
                                      false, /* dynamic */
                                      false, /* removable */
                                      false  /* stickingatnode */ );
                SCIPaddCons(scip, con);
                logical.at(i*T+t) = con;
            }
        }




        ///// Min-up /////
        char con_name_min_up[255];
        for (int i = 0 ; i <n ; i++)
        {
            for (int t = inst->getL(i); t < T; t++)
            {
                SCIP_CONS* con = NULL;
                (void) SCIPsnprintf(con_name_min_up, 255, "MinUp(%d,%d)", i, t); // nom de la contrainte
                SCIPcreateConsLinear( scip, &con, con_name_min_up, 0, NULL, NULL,
                                      0.0,   // lhs
                                      SCIPinfinity(scip),   // rhs  SCIPinfinity(scip) if >=1
                                      true,  /* initial */
                                      false, /* separate */
                                      true,  /* enforce */
                                      true,  /* check */
                                      true,  /* propagate */
                                      false, /* local */
                                      true,  /* modifiable */
                                      false, /* dynamic */
                                      false, /* removable */
                                      false  /* stickingatnode */ );
                SCIPaddCons(scip, con);
                min_up.at(i*T + t) = con;
            }
        }

        ///// Min-down /////
        char con_name_min_down[255];
        for (int i = 0 ; i <n ; i++)
        {
            for (int t = inst->getl(i); t < T; t++)
            {
                SCIP_CONS* con = NULL;
                (void) SCIPsnprintf(con_name_min_down, 255, "MinDown(%d,%d)", i, t); // nom de la contrainte
                SCIPcreateConsLinear( scip, &con, con_name_min_down, 0, NULL, NULL,
                                      -SCIPinfinity(scip),   // lhs
                                      1.0,   // rhs  SCIPinfinity(scip) if >=1
                                      true,  /* initial */
                                      false, /* separate */
                                      true,  /* enforce */
                                      true,  /* check */
                                      true,  /* propagate */
                                      false, /* local */
                                      true,  /* modifiable */
                                      false, /* dynamic */
                                      false, /* removable */
                                      false  /* stickingatnode */ );
                SCIPaddCons(scip, con);
                min_down.at(i*T + t) = con;
            }
        }

        if (Param.useUVar) {

            vector<SCIP_VAR*> u_var ;

            u_var.resize(n*T, (SCIP_VAR*) NULL) ;

            SCIP_Vartype type ;
            if (Param.IP) {
                type = SCIP_VARTYPE_INTEGER;
            }
            else {
                type = SCIP_VARTYPE_CONTINUOUS ;
            }

            char var_name[255];

            for (int i = 0 ; i <n ; i++)
            {
                for (int t = 1; t < T; t++)
                {
                    SCIP_VAR* var = NULL;

                    SCIPsnprintf(var_name, 255, "u(%d,%d)",i,t);
                    SCIPdebugMsg(scip, "Variable <%s>\n", var_name);


                    SCIPcreateVar(scip, &var, var_name,
                                  0.0,                     // lower bound
                                  1.0,      // upper bound
                                  0,                     // objective
                                  type, // variable type
                                  true, false, NULL, NULL, NULL, NULL, NULL);

                    u_var.at(i*T+t) = var ;

                    /* add new variable to scip */
                    SCIPaddVar(scip, var);

                    /* add coefficient to the logical constraint */

                    SCIPaddCoefLinear(scip, logical.at(i*T+t), var, -1.0);

                    /* add coefficients to the min up constraints */
                    int max_min_up = fmin(T-1, t + inst->getL(i) - 1) ;
                    int min_min_up = fmax(inst->getL(i), t) ;
                    // u(i,t) apparait dans les contraintes de min-up de min_min_up à max_min_up :
                    for (int k = min_min_up ; k <= max_min_up ; k++) {
                        SCIPaddCoefLinear(scip, min_up.at(i*T + k), var, -1.0);
                    }

                    /* add coefficients to the min down constraints */
                    int max_min_down = fmin(T-1, t + inst->getl(i) - 1) ;
                    int min_min_down = fmax(inst->getl(i), t);
                    // u(i,t) apparait dans les contraintes de min-down de t à max_min_down :
                    for (int k = min_min_down ; k <= max_min_down ; k++) {
                        SCIPaddCoefLinear(scip, min_down.at(i*T + k), var, 1.0);
                    }
                }
            }
        }
    }



    ///////////////////////////////////////////////////////////////
    //////////   SITE LAMBDA VARIABLES INITIALIZATION   /////////
    ///////////////////////////////////////////////////////////////

    //Add variables corresponding to "all up" up/down plans (assuming initial state is up for all units)

    L_var_site.clear();


    for (int s=0 ; s<S; s++)
    {
        //cout << "s:" << s << endl ;
        IloNumArray plan = IloNumArray(env, Param.nbUnits(s)*T) ;
        for (int index=0 ; index < Param.nbUnits(s)*T ; index++) {
            plan[index]=1 ;
        }

        int first = Param.firstUnit(s);
        //cout << "first = " << first << endl;
        Master_Variable* lambda = new Master_Variable(s, plan);
        if (Param.powerPlanGivenByLambda) {
            IloNumArray powerPlan = IloNumArray(env, Param.nbUnits(s)*T) ;
            for (int i=0 ; i< Param.nbUnits(s) ; i++) {
                for (int t=0 ; t < T ; t++) {
                    powerPlan[i*T + t]= inst->getPmax(first+i) -  inst->getPmin(first+i);
                }
            }
            lambda->addPowerPlan(powerPlan);
        }

        initMasterSiteVariable(scip, inst, lambda);
        SCIPaddVar(scip, lambda->ptr);
        addCoefsToConstraints_siteVar(scip, lambda, inst) ;
    }

    cout << "site vars created" << endl;


    ///////////////////////////////////////////////////////////////
    //////////   TIME LAMBDA VARIABLES INITIALIZATION   /////////
    ///////////////////////////////////////////////////////////////

    //Add variables corresponding to "all up" up/down plans (assuming initial state is up for all units), each unit produces at Pmax

    L_var_time.clear();

    for (int t = 0 ; t<T ; t++)
    {
        IloNumArray plan = IloNumArray(env, n) ;
        for (int index=0 ; index < n ; index++) {
            plan[index]=1 ;
        }
        MasterTime_Variable* lambda = new MasterTime_Variable(t, plan);
        if (Param.powerPlanGivenByMu) {
            IloNumArray powerPlan = IloNumArray(env, n) ;
            for (int i=0 ; i < n ; i++) {
                powerPlan[i] = inst->getPmax(i);
            }
            lambda->addPowerPlan(powerPlan);
        }
        initMasterTimeVariable(scip, lambda);

        SCIPaddVar(scip, lambda->ptr);

        addCoefsToConstraints_timeVar(scip, lambda) ;
    }


}

void MasterDouble_Model::computeFracSol(SCIP* scip) {
    list<Master_Variable*>::const_iterator itv;
    SCIP_Real frac_value;
    for (int ind=0 ; ind < n*T ; ind++) {
        x_frac[ind]=0;
    }

    for (itv = L_var_site.begin(); itv!=L_var_site.end(); itv++) {

        frac_value = fabs(SCIPgetVarSol(scip,(*itv)->ptr));

        int site = (*itv)->Site ;
        int first = Param.firstUnit(site) ;
        for (int i=0 ; i < Param.nbUnits(site) ; i++) {
            //int group = inst->getGroup(first+i)  ;
            for (int t=0 ; t < T ; t++) {
                if ((*itv)->UpDown_plan[i*T+t] > Param.Epsilon) {
                    //group_frac[group*T + t] += frac_value ;
                    x_frac[(first+i)*T+t] += frac_value ;
                }
            }
        }
    }
}


void MasterDouble_Model::discardVar(SCIP* scip, SCIP_ConsData* consdata) {

//    /////On met à 0 les lambda incompatibles avec la contrainte

   consdata->L_var_bound.clear() ; // L_var_bound stocke les variables scip dont la borne a été effectivement changée (ie elle n'était pas déjà à 0)

   list<Master_Variable*>::const_iterator itv_site;

   for (itv_site = L_var_site.begin(); itv_site!=L_var_site.end(); itv_site++) {
       if ((*itv_site)->Site == consdata->site) {
           if ((*itv_site)->UpDown_plan[consdata->unit*T + consdata->time] != consdata->bound ) {

               SCIP_Real old_bound =  SCIPgetVarUbAtIndex(scip, (*itv_site)->ptr, NULL, 0) ;

               ///  L_var_bound est mis à jour
               if (!SCIPisZero(scip,old_bound)) {
                   SCIPchgVarUbNode(scip, NULL, (*itv_site)->ptr, 0) ;
                   consdata->L_var_bound.push_back((*itv_site)->ptr) ;
               }
           }
       }
   }

   list<MasterTime_Variable*>::const_iterator itv_time;

     for (itv_time = L_var_time.begin(); itv_time!=L_var_time.end(); itv_time++) {
         if ((*itv_time)->time == consdata->time) {
             if ((*itv_time)->UpDown_plan[consdata->unit] != consdata->bound ) {

                 SCIP_Real old_bound =  SCIPgetVarUbAtIndex(scip, (*itv_time)->ptr, NULL, 0) ;

                 ///  L_var_bound est mis à jour
                 if (!SCIPisZero(scip,old_bound)) {
                     SCIPchgVarUbNode(scip, NULL, (*itv_time)->ptr, 0) ;
                     consdata->L_var_bound.push_back((*itv_time)->ptr) ;
                 }
             }
         }
     }
}

void MasterDouble_Model::restoreVar(SCIP* scip, SCIP_ConsData* consdata) {

   ////On remet à +inf les lambda qui étaient incompatibles avec la contrainte de branchement

   list<SCIP_VAR*>::const_iterator itv;

   for (itv = consdata->L_var_bound.begin(); itv!=consdata->L_var_bound.end(); itv++) {
       SCIPchgVarUbNode(scip, NULL, (*itv), SCIPinfinity(scip)) ;
   }
    consdata->L_var_bound.clear() ;

}


////////// Créé des variables lambda à partir d'une solution (x) ///////////
void MasterDouble_Model::createColumns(SCIP* scip, IloNumArray x, IloNumArray p) {

    int T = inst->getT() ;

    for (int s = 0 ; s < Param.nbDecGpes ; s++) {
        int size=Param.nbUnits(s);

        IloNumArray plan = IloNumArray(env, T*size) ;
        int first = Param.firstUnit(s);
        for (int i=0 ; i < size ; i++) {
            for (int t=0 ; t < T ; t++) {
                if (x[(first+i)*T + t] > 1 - Param.Epsilon) {
                    plan[i*T+t]=1 ;
                }
                if (x[(first+i)*T + t] < Param.Epsilon) {
                    plan[i*T+t]=0 ;
                }
            }
        }

        Master_Variable* lambda = new Master_Variable(s, plan);
        initMasterSiteVariable(scip, inst, lambda);

        if (!Param.powerPlanGivenByLambda) { //fonction spécifique à implémenter dans le cas où le paramètre est à 1
            SCIPaddVar(scip, lambda->ptr);
            addCoefsToConstraints_siteVar(scip, lambda, inst) ;
        }
    }


    /// COLONNES TIME
    int n = inst->getn() ;

    for (int t = 0 ; t < T ; t++) {

        IloNumArray plan = IloNumArray(env, n) ;
        for (int i=0 ; i < n ; i++) {
            if (x[i*T + t] > 1 - Param.Epsilon) {
            plan[i]=1 ;
            }
            if (x[i*T + t] < Param.Epsilon) {
            plan[i]=0 ;
            }
        }

        MasterTime_Variable* lambda = new MasterTime_Variable(t, plan) ;

        if (Param.powerPlanGivenByMu){
            IloNumArray powerPlan = IloNumArray(env, n) ;
            for (int i=0 ; i < n ; i++) {
                powerPlan[i] = p[i*T+t];
            }
            lambda->addPowerPlan(powerPlan);
        }

        initMasterTimeVariable(scip, lambda);

        SCIPaddVar(scip, lambda->ptr);

        addCoefsToConstraints_timeVar(scip, lambda) ;
    }
}
