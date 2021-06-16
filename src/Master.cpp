#include "Master.h"

/* namespace usage */
using namespace std;
using namespace scip;

Master_Variable::Master_Variable(int site, IloNumArray UpDown) {
    ptr = NULL ;
    Site = site ;
    cost = 0 ;
    UpDown_plan = UpDown  ;
}

void Master_Variable::addPowerPlan(IloNumArray PowerPlan) {

    Power_plan = PowerPlan  ;
}

void Master_Variable::computeCost(InstanceUCP* inst, const Parameters & Param) {
    //compute cost of up/down plan lambda: fixed cost (including minimum power output cost) and start up cost
    //init à prendre en compte plus tard
    cost=0 ;

    int T= inst->getT() ;

    int first = Param.firstUnit(Site) ;
    int last = Param.firstUnit(Site) + Param.nbUnits(Site) - 1 ;


    // cout << "site " << Site << ", first unit : " << first << ", nb units: " << Param.nbUnits(Site) << endl ;

    for (int i = first ; i <= last ; i++) {
        int down_t_1 = 0 ;
        for (int t = 0 ; t < T ; t++) {
            if (down_t_1 && UpDown_plan[(i-first)*T+t] > 1 - Param.Epsilon) { // il y a un démarrage en t
                cost+= inst->getc0(i) ;
            }

            if (UpDown_plan[(i-first)*T+t] < Param.Epsilon ) {
                down_t_1=1;
            }
            else { // l'unité i est up en t
                cost+= inst->getcf(i) + inst->getcp(i)*inst->getPmin(i) ;
                down_t_1=0;
            }
        }
    }

    if (Param.powerPlanGivenByLambda) {
        for (int i = first ; i <= last ; i++) {
            for (int t = 0 ; t < T ; t++) {
                cost += Power_plan[(i-first)*T+t]*inst->getcp(i) ;
            }
        }
    }
}


void MasterSite_Model::addCoefsToConstraints(SCIP* scip, Master_Variable* lambda, InstanceUCP* inst) {

    int s = lambda->Site ;
    int first = Param.firstUnit(s) ;

    /* for each time period, add coefficient pmin into the demand constraint at t, for each unit unit i up in site S */
    for (int t=0 ; t < T ; t++) {
        for (int i=0 ; i < Param.nbUnits(s) ; i++) {

            if (!Param.powerPlanGivenByLambda) {
                if (lambda->UpDown_plan[i*T+t] > 1 - Param.Epsilon) {
                    SCIPaddCoefLinear(scip, demand_cstr[t], lambda->ptr, inst->getPmin(first+i)) ;
                }
            }

            else {
                double power = lambda->Power_plan[i*T+t] + lambda->UpDown_plan[i*T+t]*inst->getPmin(first+i) ;
                SCIPaddCoefLinear(scip, demand_cstr[t], lambda->ptr, power) ;
            }
        }
    }

    /* for each time period and each unit in site S, add coefficient pmin(i) - pmax(i) into the power limit constraint of unit i at t */
    for (int t=0 ; t < T ; t++) {
        for (int i=0 ; i < Param.nbUnits(s) ; i++) {

            if (!Param.powerPlanGivenByLambda) {
                if (lambda->UpDown_plan[i*T+t] > 1 - Param.Epsilon) {
                    SCIPaddCoefLinear(scip, power_limits[(first+i)*T+t], lambda->ptr, inst->getPmax(first+i) - inst->getPmin(first+i)) ;
                }
            }
        }
    }

    /* for each time period and each unit in site S, add coefficient RU (resp RD) into the ramp up (resp down) constraint of unit i at t+1 */
    //RAMPSTUFF
    if (Param.Ramp && Param.rampInMaster && !Param.powerPlanGivenByLambda) {

        for (int i=0 ; i < Param.nbUnits(s) ; i++) {
            double RU = (inst->getPmax(first+i) - inst->getPmin(first+i))/3 ;
            double RD = (inst->getPmax(first+i) - inst->getPmin(first+i))/2 ;

            for (int t=1 ; t < T ; t++) {
                if (lambda->UpDown_plan[i*T+t-1]  > 1 - Param.Epsilon) {
                    SCIPaddCoefLinear(scip, ramp_up[(first+i)*T+t], lambda->ptr, -RU) ;
                }
                if (lambda->UpDown_plan[i*T+t]  > 1 - Param.Epsilon) {
                    SCIPaddCoefLinear(scip, ramp_down[(first+i)*T+t], lambda->ptr, -RD) ;
                }

            }
        }
    }


    /* add coefficient to the convexity constraint for site s */
    SCIPaddCoefLinear(scip, convexity_cstr[s], lambda->ptr, 1.0) ;



    ///INTRASITE
    if (Param.IntraSite && Param.UnitDecompo) {
        // intrasite constraint in the master

        int site = inst->getSiteOf(s) ; // s est un singleton ici (c'est le groupe d'unité sur lequel on décompose) = une unité

        for (int t=1 ; t <T ; t++) {
            if ( (lambda->UpDown_plan[t]  > 1 - Param.Epsilon) && (lambda->UpDown_plan[t-1] < Param.Epsilon) ) { // si unité s démarre en t
                SCIPaddCoefLinear(scip, intrasite.at(site*T+t), lambda->ptr, 1.0) ;

            }
        }
    }

    /// CONTRAINTES LIEES A LA DECOMPO START UP
    if (Param.StartUpDecompo) {

        for (int i=0 ; i < Param.nbUnits(s) ; i++) {
            for (int t=0 ; t <T ; t++) {

                int a_t = (lambda->UpDown_plan[i*T+t]  > 1 - Param.Epsilon) ;
                int a_t_1=0;
                int b_t=0;
                if (t >= 1) {
                    a_t_1 = (lambda->UpDown_plan[i*T+t-1]  > 1 - Param.Epsilon) ;
                    b_t = ( (lambda->UpDown_plan[i*T+t]  > 1 - Param.Epsilon) && (lambda->UpDown_plan[i*T + t-1] < Param.Epsilon) )  ;
                }

                int L = inst->getL(first+i) ;

                //Min down
                if ( b_t ) {
                    int kmin = fmax(t, L) ;
                    int kmax = fmin(t+L-1, T-1);
                    for (int k=kmin ; k<=kmax; k++) {
                        SCIPaddCoefLinear(scip, mindown.at((first+i)*T+k), lambda->ptr, 1.0) ;
                    }
                }
                if (t >= L && a_t) {
                    SCIPaddCoefLinear(scip, mindown.at((first+i)*T+t), lambda->ptr, -1.0) ;
                }


                //z_lambda
                if ( a_t ) {
                    SCIPaddCoefLinear(scip, z_lambda[(first+i)*T+t], lambda->ptr, 1.0) ;
                }

                //logical
                if (t >= 1) {
                    SCIPaddCoefLinear(scip, logical[(first+i)*T+t], lambda->ptr, a_t - a_t_1 - b_t) ;
                }

            }
        }

    }
}


void MasterSite_Model::initMasterVariable(SCIP* scip, InstanceUCP* inst , Master_Variable* var) {
    char var_name[255];
    SCIPsnprintf(var_name, 255, "V_%d",L_var.size());
    SCIPdebugMsg(scip, "new variable <%s>\n", var_name);

    /* create the new variable: Use upper bound of infinity such that we do not have to care about
     * the reduced costs of the variable in the pricing. The upper bound of 1 is implicitly satisfied
     * due to the set partitioning constraints.
     */

    var->computeCost(inst, Param);
    double cost= var->cost;

    cout << "cost: " << cost << endl ;

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
    L_var.push_back(var);


    //    cout << "Variable " << var_name << " added, with plan:" << endl ;
    //    for (int t=0 ; t < T ; t++) {
    //        for (int i=0 ; i < Param.nbUnits(var->Site) ; i++) {
    //            cout << var->UpDown_plan[i*T+t] << " " ;
    //        }
    //        cout << endl ;
    //    }
}

MasterSite_Model::MasterSite_Model(InstanceUCP* inst, const Parameters & Parametres) : Master_Model(Parametres, inst) {
    n = inst->getn() ;
    T = inst->getT() ;


    S = Parametres.nbDecGpes ;



    cumul_resolution_pricing= 0 ;
    demand_cstr.resize(T, (SCIP_CONS*) NULL);
    power_limits.resize(n*T, (SCIP_CONS*) NULL) ;
    ramp_up.resize(n*T, (SCIP_CONS*) NULL) ;
    ramp_down.resize(n*T, (SCIP_CONS*) NULL) ;
    convexity_cstr.resize(S, (SCIP_CONS*) NULL) ;
    intrasite.resize(inst->getS()*T, (SCIP_CONS*) NULL) ;

    mindown.resize(n*T, (SCIP_CONS*) NULL) ;
    z_lambda.resize(n*T, (SCIP_CONS*) NULL) ;
    logical.resize(n*T, (SCIP_CONS*) NULL) ;
}

void  MasterSite_Model::InitScipMasterModel(SCIP* scip, InstanceUCP* inst) {


    ////////////////////////////////////////////////////////////////
    /////////////   MASTER CONSTRAINT INITIALIZATION   /////////////
    ////////////////////////////////////////////////////////////////
    // Constraints form: lhs <= ax <= rhs


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



    ///// Intra-site /////

    if (Param.UnitDecompo && Param.IntraSite) {
        char con_name_intrasite[255];
        for (int site = 0 ; site < inst->getS() ; site++) {
            for (int t = 1; t < T; t++)   {
                SCIP_CONS* con = NULL;
                (void) SCIPsnprintf(con_name_intrasite, 255, "Intrasite(%d,%d)", site,t); // nom de la contrainte
                SCIPcreateConsLinear( scip, &con, con_name_intrasite, 0, NULL, NULL,
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
                intrasite[site*T + t] = con;
            }
        }
    }

    ///// Ramp up & down constraints /////
    //RAMPSTUFF
    if (Param.Ramp && Param.rampInMaster) {
        char con_name_ramp_up[255];
        for (int i = 0 ; i <n ; i++)
        {
            for (int t = 1; t < T; t++)
            {
                SCIP_CONS* con = NULL;
                (void) SCIPsnprintf(con_name_ramp_up, 255, "RampUp(%d,%d)", i, t); // nom de la contrainte
                SCIPcreateConsLinear( scip, &con, con_name_ramp_up, 0, NULL, NULL,
                                      -SCIPinfinity(scip),   // lhs
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
                ramp_up[i*T + t] = con;
            }
        }

        char con_name_ramp_down[255];
        for (int i = 0 ; i <n ; i++)
        {
            for (int t = 1; t < T; t++)
            {
                SCIP_CONS* con = NULL;
                (void) SCIPsnprintf(con_name_ramp_down, 255, "RampDown(%d,%d)", i, t); // nom de la contrainte
                SCIPcreateConsLinear( scip, &con, con_name_ramp_down, 0, NULL, NULL,
                                      -SCIPinfinity(scip),   // lhs
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
                ramp_down[i*T + t] = con;
            }
        }
    }

    cout << "convex cons" << endl ;

    ///// Convexity constraint ////
    char con_name_convex[255];
    for (int s = 0 ; s<S ; s++)
    {
        SCIP_CONS* con = NULL;
        (void) SCIPsnprintf(con_name_convex, 255, "Convexity(%d)", s); // nom de la contrainte
        SCIPcreateConsLinear( scip, &con, con_name_convex, 0, NULL, NULL,
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
        convexity_cstr[s] = con;
    }



    if (Param.StartUpDecompo) {

        /// Min down constraint
        char con_name_mindown[255];
        for (int i = 0 ; i <n ; i++)
        {
            for (int t = 0; t < T; t++)
            {
                SCIP_CONS* con = NULL;
                (void) SCIPsnprintf(con_name_mindown, 255, "MinDown(%d,%d)", i, t); // nom de la contrainte
                SCIPcreateConsLinear( scip, &con, con_name_mindown, 0, NULL, NULL,
                                      -SCIPinfinity(scip),   // lhs
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
                mindown[i*T + t] = con;
            }
        }

        /// z _ lambda constraint
        char con_name_z_lambda[255];
        for (int i = 0 ; i <n ; i++)
        {
            for (int t = 0; t < T; t++)
            {
                SCIP_CONS* con = NULL;
                (void) SCIPsnprintf(con_name_z_lambda, 255, "z_lambda(%d,%d)", i, t); // nom de la contrainte
                SCIPcreateConsLinear( scip, &con, con_name_z_lambda, 0, NULL, NULL,
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
                z_lambda[i*T + t] = con;
            }
        }

        /// logical constraint
        char con_name_logical[255];
        for (int i = 0 ; i <n ; i++)
        {
            for (int t = 0; t < T; t++)
            {
                SCIP_CONS* con = NULL;
                (void) SCIPsnprintf(con_name_logical, 255, "Logical(%d,%d)", i, t); // nom de la contrainte
                SCIPcreateConsLinear( scip, &con, con_name_logical, 0, NULL, NULL,
                                      -SCIPinfinity(scip),   // lhs
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
                logical[i*T + t] = con;
            }
        }


    } // fin if StartUpDecompo

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
                SCIPaddCoefLinear(scip, demand_cstr[t], var, 1.0);

                /* add coefficient to the power limit constraint */
                SCIPaddCoefLinear(scip, power_limits[i*T + t], var, -1.0);


                /* add coefficients to the ramp up and down constraints */

                //RAMPSTUFF
                if (Param.Ramp && Param.rampInMaster) {
                    if (t>0) {
                        SCIPaddCoefLinear(scip, ramp_up[i*T + t], var, 1.0);
                        SCIPaddCoefLinear(scip, ramp_down[i*T + t], var, -1.0);
                    }
                    if (t < T-1) {
                        SCIPaddCoefLinear(scip, ramp_up[i*T + t+1], var, -1.0);
                        SCIPaddCoefLinear(scip, ramp_down[i*T + t+1], var, 1.0);
                    }
                }
            }
        }
    }

    cout << "fin" << endl ;

    ///////////////////////////////////////////////////////////////
    //////////   MASTER LAMBDA VARIABLES INITIALIZATION   /////////
    ///////////////////////////////////////////////////////////////

    //Add variables corresponding to "all up" up/down plans (assuming initial state is up for all units)

    L_var.clear();


    for (int s=0 ; s<S; s++)
    {
        IloNumArray plan = IloNumArray(env, Param.nbUnits(s)*T) ;
        for (int index=0 ; index < Param.nbUnits(s)*T ; index++) {
            plan[index]=1 ;
        }

        int first = Param.firstUnit(s);
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


        initMasterVariable(scip, inst, lambda);

        SCIPaddVar(scip, lambda->ptr);

        addCoefsToConstraints(scip, lambda, inst) ;
    }


    /////////////////////////////////////////////////////////////
    ////////   MASTER Z VARIABLES INITIALIZATION   //////////
    /////////////////////////////////////////////////////////////
    /// --> Variables z from start up decomposition


    SCIP_Vartype type ;
    if (Param.IP) {
        type = SCIP_VARTYPE_INTEGER;
    }
    else {
        type = SCIP_VARTYPE_CONTINUOUS ;
    }


    // Création variables z et ajout dans les contraintes
    if (Param.StartUpDecompo) {
        char var_name[255];

        for (int i = 0 ; i <n ; i++)
        {
            for (int t = 0; t < T; t++)
            {
                SCIP_VAR* var = NULL;

                SCIPsnprintf(var_name, 255, "z(%d,%d)",i,t);
                SCIPdebugMsg(scip, "Variable <%s>\n", var_name);


                SCIPcreateVar(scip, &var, var_name,
                              0.0,                     // lower bound
                              1.0,      // upper bound
                              -inst->getcf(i) - inst->getcp(i)*inst->getPmin(i),                     // objective
                              type, // variable type
                              true, false, NULL, NULL, NULL, NULL, NULL);


                /* add new variable to scip */
                SCIPaddVar(scip, var);

                /* add coefficient to the demand constraint */
                SCIPaddCoefLinear(scip, demand_cstr[t], var, -inst->getPmin(i));

                /* add coefficient to the power limit constraint */
                SCIPaddCoefLinear(scip, power_limits[i*T + t], var, -(inst->getPmax(i) - inst->getPmin(i)));

                /* add coef to min down constraint */
                if (t >= inst->getL(i)) {
                    SCIPaddCoefLinear(scip, mindown[i*T + t], var, 1.0);
                }


                /* add coef to z_lambda constraint */
                SCIPaddCoefLinear(scip, z_lambda[i*T + t], var, -1.0);

                /* add coef to logical constraints */
                if (t >= 1) {
                    // logical constraint do not exist for t=0
                    SCIPaddCoefLinear(scip, logical[i*T + t], var, -1.0);
                }
                if (t < T-1) {
                    SCIPaddCoefLinear(scip, logical[i*T + t+1], var, 1.0);
                }


                /* add coef to ramp up/down*/
                ///todo
            }
        }
    }


}


void MasterSite_Model::computeFracSol(SCIP* scip) {
    list<Master_Variable*>::const_iterator itv;
    SCIP_Real frac_value;
    for (int ind=0 ; ind < n*T ; ind++) {
        x_frac[ind]=0;
    }

    for (itv = L_var.begin(); itv!=L_var.end(); itv++) {

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


void MasterSite_Model::discardVar(SCIP* scip, SCIP_ConsData* consdata) {

    /////On met à 0 les lambda incompatibles avec la contrainte

    consdata->L_var_bound.clear() ; // L_var_bound stocke les variables scip dont la borne a été effectivement changée (ie elle n'était pas déjà à 0)

    list<Master_Variable*>::const_iterator itv;

    for (itv = L_var.begin(); itv!=L_var.end(); itv++) {
        if ((*itv)->Site == consdata->site) {
            if ((*itv)->UpDown_plan[consdata->unit*T + consdata->time] != consdata->bound ) {

                SCIP_Real old_bound =  SCIPgetVarUbAtIndex(scip, (*itv)->ptr, NULL, 0) ;

                ///  L_var_bound est mis à jour
                if (!SCIPisZero(scip,old_bound)) {
                    SCIPchgVarUbNode(scip, NULL, (*itv)->ptr, 0) ;
                    consdata->L_var_bound.push_back((*itv)->ptr) ;
                }
            }
        }
    }
}

void MasterSite_Model::restoreVar(SCIP* scip, SCIP_ConsData* consdata) {

    ////On remet à +inf les lambda qui étaient incompatibles avec la contrainte de branchement

    list<SCIP_VAR*>::const_iterator itv;

    for (itv = consdata->L_var_bound.begin(); itv!=consdata->L_var_bound.end(); itv++) {
        SCIPchgVarUbNode(scip, NULL, (*itv), SCIPinfinity(scip)) ;
    }
    consdata->L_var_bound.clear() ;

}


//////// Créé des variables lambda à partir d'une solution (x) ///////////
void MasterSite_Model::createColumns(SCIP* scip, IloNumArray x) {

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
        initMasterVariable(scip, inst, lambda);

        if (!Param.powerPlanGivenByLambda) { //fonction spécifique à implémenter dans le cas où le paramètre est à 1
            SCIPaddVar(scip, lambda->ptr);
            addCoefsToConstraints(scip, lambda, inst) ;
        }
    }
}
