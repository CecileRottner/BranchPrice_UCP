#include "Master.h"

/* namespace usage */
using namespace std;
using namespace scip;


/////////////////////////////////////
////////// INTERVAL UP SET //////////
/////////////////////////////////////


IneqIntUpSet::IneqIntUpSet(SCIP* scip, const Parameters & Param, int num, int al, list<int> *C_ptr, int ii, int tt0, int tt1) :
    alpha(al),
    i(ii),
    t0(tt0),
    t1(tt1)
{
    C = C_ptr;
    dual=0 ;
    ineq = NULL;
    char con_name_iup[255];
    (void) SCIPsnprintf(con_name_iup, 255, "iup(%d)", num); // nom de la contrainte

    bool modifiable = true ;

    SCIPcreateConsLinear( scip, &ineq, con_name_iup, 0, NULL, NULL,
                          alpha,   // lhs
                          SCIPinfinity(scip),   // rhs  SCIPinfinity(scip) if >=1
                          true,  /* initial */
                          true, /* separate */
                          true,  /* enforce */
                          true,  /* check */
                          true,  /* propagate */
                          false, /* local */
                          modifiable,  /* modifiable */
                          false, /* dynamic */
                          false, /* removable */
                          false  /* stickingatnode */ );
}


void MasterTime_Model::addIntUpSet(SCIP* scip, IneqIntUpSet* Iup) {

    SCIPaddCons(scip, Iup->ineq);
    cout << "iup added" << endl;
    nbIntUpSet++ ;

    int t0 = Iup->t0 ;
    int t1 = Iup->t1 ;
    int i = Iup->i ;

    (IUP_t0.at(t0)).push_back(Iup) ;
    (IUP_t1.at(t1)).push_back(Iup) ;

    double violation = -Iup->alpha ;
    //Initialisation (coefs des variables u)
    for (int t=t0+1 ; t <= t1 ; t++) {
        SCIPaddCoefLinear(scip, Iup->ineq, u_var.at(i*T+t), -1.0) ;
        violation -= fabs(SCIPgetVarSol(scip,u_var[i*T+t])) ;

        list<int>::const_iterator j;
        for (j=Iup->C->begin() ; j != Iup->C->end() ; j++) {
            SCIPaddCoefLinear(scip, Iup->ineq, u_var.at((*j)*T+t), 1.0) ;
            violation += fabs(SCIPgetVarSol(scip,u_var[(*j)*T+t])) ;
        }
    }

    //Ajout des coefs des lambda
    list<MasterTime_Variable*>::const_iterator itv;
    for (itv = L_var.begin(); itv!=L_var.end(); itv++) {

        int time = (*itv)->time ;
        if (time == t1) {
            if ((*itv)->UpDown_plan[i] > 1 - Param.Epsilon) {
                SCIPaddCoefLinear(scip, Iup->ineq, (*itv)->ptr, 1.0) ;
                violation += fabs(SCIPgetVarSol(scip,(*itv)->ptr)) ;
            }
        }

        if (time == t0) {

            list<int>::const_iterator j;
            for (j=Iup->C->begin() ; j != Iup->C->end() ; j++) {

                if ((*itv)->UpDown_plan[(*j)] > 1 - Param.Epsilon) {
                    SCIPaddCoefLinear(scip, Iup->ineq, (*itv)->ptr, 1.0) ;
                    violation += fabs(SCIPgetVarSol(scip,(*itv)->ptr)) ;
                }
            }
        }
    }
    cout << "violation: " << violation << endl ;
}


///////////////////////////////
////////// VARIABLES //////////
///////////////////////////////


////// Constructeur //////
MasterTime_Variable::MasterTime_Variable(int t, IloNumArray UpDown) {
    ptr = NULL ;
    time = t ;
    UpDown_plan = UpDown  ;
}

void MasterTime_Variable::addPowerPlan(IloNumArray PowerPlan) {

    Power_plan = PowerPlan  ;
}

void MasterTime_Variable::computeCost(InstanceUCP* inst, const Parameters & Param) {
    cost = 0;

    int n = inst->getn() ;
    if (Param.doubleDecompo){
        for (int i=0 ; i < n ; i++) {
            if (Param.powerPlanGivenByMu){
                cost += Power_plan[i] * inst->getcp(i) ;
                if (UpDown_plan[i] > 1 - Param.Epsilon){
                    cost += (1 - Param.costBalancingMaster.at(i)) * inst->getcf(i) ;
                    if (Param.PminOnLambda ) {
                        cost -= Param.costBalancingMaster.at(i) * inst->getPmin(i) * inst->getcp(i) ;
                    }
                    if (Param.PmaxOnLambda ) {
                        cost -= Param.costBalancingMaster.at(i) * inst->getPmax(i) * inst->getcp(i) ;
                    } 
                }
            }
            else {
                if (UpDown_plan[i] > 1 - Param.Epsilon){
                    cost += (1 - Param.costBalancingMaster.at(i)) * inst->getcf(i) ;
                    if (Param.PminOnLambda ) {
                        cost += (1 - Param.costBalancingMaster.at(i)) * inst->getPmin(i) * inst->getcp(i) ;
                    }
                    if (Param.PmaxOnLambda ) {
                        cost += (1 - Param.costBalancingMaster.at(i)) * inst->getPmax(i) * inst->getcp(i) ;
                    }
                }
            }
        }
    }
    else{
        for (int i=0 ; i < n ; i++) {
            if (UpDown_plan[i] > 1 - Param.Epsilon){
                cost += inst->getcf(i) ;
                cost += Power_plan[i] * inst->getcp(i) ;
            }
        }
    }
}

/////// ajout des coefficients dans chaque contrainte pour variable lambda //////////
void MasterTime_Model::addCoefsToConstraints(SCIP* scip, MasterTime_Variable* lambda) {

    int t = lambda->time ;

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

    /* add coefficient to the convexity constraint for site s */
    SCIPaddCoefLinear(scip, convexity_cstr.at(t), lambda->ptr, 1.0) ;

    ///SSBI coefficients

    if (Param.masterSSBI) {
        for (int i=0; i < n-1 ; i++) {

            //// RSU ////
            if (!inst->getLast(i)) {
                int l = inst->getl(i);
                if (lambda->UpDown_plan[i] > 1 - Param.Epsilon) {
                    if (t<=T-1-l) {
                        SCIPaddCoefLinear(scip, rsu.at(i*T+t+l), lambda->ptr, 1.0) ;
                    }
                }
            }

            ///// RSD ////
            if (!Param.RSUonly) {
            int L = inst->getL(i) ;
            if (!inst->getLast(i)) {

                /// Coef RSD(i,t)
                if (t >= L && i<n-1) {
                    double a_i = 0 ;
                    double a_iP1 = 0 ;

                    if (lambda->UpDown_plan[i] > 1 - Param.Epsilon) {
                        a_i=1 ;
                    }

                    if (lambda->UpDown_plan[i+1] > 1 - Param.Epsilon) {
                        a_iP1 = 1 ;
                    }

                    SCIPaddCoefLinear(scip, rsd.at(i*T+t), lambda->ptr, a_i - a_iP1) ;
                }

                /// Coef RSD(i,t+1)
                if (t < T-1 && t + 1 >= L && i<n-1) {
                    if (lambda->UpDown_plan[i] > 1 - Param.Epsilon) {
                        SCIPaddCoefLinear(scip, rsd.at(i*T+t+1), lambda->ptr, -1.0) ;
                    }
                }
            }
            }

        } //// fin for i
    } ///// fin SSBI


    //// Interval up set inequalities ////

    if (Param.IntervalUpSet && nbIntUpSet>0) {

        list<IneqIntUpSet*>::const_iterator iup;

        // Int-up-set telles que t1=time
        for (iup = IUP_t1[t].begin(); iup!= IUP_t1[t].end() ; iup++) {
            int i = (*iup)->i ;
            if (lambda->UpDown_plan[i] > 1 - Param.Epsilon) {
                SCIPaddCoefLinear(scip, (*iup)->ineq, lambda->ptr, 1.0) ;
            }
        }

        // Int-up-set telles que t0=time
        for (iup = IUP_t0[t].begin(); iup!= IUP_t0[t].end() ; iup++) {
            list<int>* C = (*iup)->C ;

            list<int>::const_iterator j;
            for (j=C->begin() ; j != C->end() ; j++) {

                if (lambda->UpDown_plan[(*j)] > 1 - Param.Epsilon) {
                    SCIPaddCoefLinear(scip, (*iup)->ineq, lambda->ptr, 1.0) ;
                }
            }
        }
    }
}


//////// Initialisation d'une variable lambda /////////////
void MasterTime_Model::initMasterTimeVariable(SCIP* scip, MasterTime_Variable* var) {

    char var_name[255];
    SCIPsnprintf(var_name, 255, "V_%d",L_var.size());
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
    L_var.push_back(var);


    cout << "Variable " << var_name << " added, with plan: "  ;

    for (int i=0 ; i < inst->getn() ; i++) {
        cout << var->UpDown_plan[i] << " "  ;
    }
    cout << endl ;
}

//////// Créé des variables lambda à partir d'une solution (x,p) ///////////
void MasterTime_Model::createColumns(SCIP* scip, IloNumArray x, IloNumArray p) {

    int n = inst->getn() ;
    int T = inst->getT() ;

    for (int t = 0 ; t < T ; t++) {
        IloNumArray plan = IloNumArray(env, n) ;
        IloNumArray powerPlan = IloNumArray(env, n) ;
        for (int i=0 ; i < n ; i++) {
            if (x[i*T + t] > 1 - Param.Epsilon) {
                plan[i]=1 ;
            }
            if (x[i*T + t] < Param.Epsilon) {
                plan[i]=0 ;
            }
            powerPlan[i] = p[i*T + t]  + inst->getPmin(i);
        }

        MasterTime_Variable* lambda = new MasterTime_Variable(t, plan) ;
        lambda->addPowerPlan(powerPlan);
        initMasterTimeVariable(scip, lambda);

        SCIPaddVar(scip, lambda->ptr);

        addCoefsToConstraints(scip, lambda) ;
    }
}

///////////////////////////////////////////
////////// INITIALISATION MASTER //////////
///////////////////////////////////////////

MasterTime_Model::MasterTime_Model(InstanceUCP* instance, const Parameters & Parametres) : Master_Model(Parametres, instance) {


    u_var.resize(n*T, (SCIP_VAR*) NULL) ;

    logical.resize(n*T, (SCIP_CONS*) NULL) ;
    min_up.resize(n*T, (SCIP_CONS*) NULL) ;
    min_down.resize(n*T, (SCIP_CONS*) NULL) ;

    convexity_cstr.resize(T, (SCIP_CONS*) NULL) ;
    intrasite.resize(inst->getS()*T, (SCIP_CONS*) NULL) ;
    rsu.resize(n*T, (SCIP_CONS*) NULL) ;
    rsd.resize(n*T, (SCIP_CONS*) NULL) ;

}

void  MasterTime_Model::initScipMasterTimeModel(SCIP* scip) {

    ////////////////////////////////////////////////////////////////
    /////////////   MASTER CONSTRAINT INITIALIZATION   /////////////
    ////////////////////////////////////////////////////////////////
    // Constraints form: lhs <= ax <= rhs


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


    ///// Convexity constraint ////
    char con_name_convex[255];
    for (int t = 0 ; t<T ; t++)
    {
        SCIP_CONS* con = NULL;
        (void) SCIPsnprintf(con_name_convex, 255, "Convexity(%d)", t); // nom de la contrainte
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
        convexity_cstr.at(t) = con;
    }



    ///// Intrasite ////

    if (Param.IntraSite) {
        char con_name_intrasite[255];
        for (int t =1 ; t<T ; t++) {
            for (int s=0 ; s < inst->getS() ; s++) {
                SCIP_CONS* con = NULL;
                (void) SCIPsnprintf(con_name_intrasite, 255, "Intrasite(%d,%d)", s, t); // nom de la contrainte
                SCIPcreateConsLinear( scip, &con, con_name_intrasite, 0, NULL, NULL,
                                      -SCIPinfinity(scip),   // lhs
                                      1.0,   // rhs  SCIPinfinity(scip) if >=1
                                      true,  /* initial */
                                      false, /* separate */
                                      true,  /* enforce */
                                      true,  /* check */
                                      true,  /* propagate */
                                      false, /* local */
                                      false,  /* modifiable */
                                      false, /* dynamic */
                                      false, /* removable */
                                      false  /* stickingatnode */ );
                SCIPaddCons(scip, con);
                intrasite.at(s*T+t) = con;
            }
        }
    }

    if (Param.masterSSBI) {

        ///// Ready to start up /////
        char con_name_rsu[255];
        for (int i = 0 ; i <n-1 ; i++)
        {
            if (!inst->getLast(i)) {
                for (int t = inst->getl(i); t < T; t++)
                {
                    SCIP_CONS* con = NULL;
                    (void) SCIPsnprintf(con_name_rsu, 255, "RSU(%d,%d)", i, t); // nom de la contrainte
                    SCIPcreateConsLinear( scip, &con, con_name_rsu, 0, NULL, NULL,
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
                    rsu.at(i*T + t) = con;
                }
            }
        }

        if (!Param.RSUonly) {
        ///// Ready to shut down /////
        char con_name_rsd[255];
        for (int i = 0 ; i <n ; i++)
        {
            if (!inst->getLast(i)) {
                for (int t = inst->getL(i); t < T; t++)
                {
                    SCIP_CONS* con = NULL;
                    (void) SCIPsnprintf(con_name_rsd, 255, "RSD(%d,%d)", i, t); // nom de la contrainte
                    SCIPcreateConsLinear( scip, &con, con_name_rsd, 0, NULL, NULL,
                                          -1.0,   // lhs
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
                    rsd.at(i*T + t) = con;
                }
            }
        }
        }
    }




    /////////////////////////////////////////////////
    ////////   MASTER START UP VARIABLES   //////////
    /////////////////////////////////////////////////

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
                          inst->getc0(i),                     // objective
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

            /* add coefficients to intrasite constraints */
            if (Param.IntraSite) {
                int site = inst->getSiteOf(i) ;
                SCIPaddCoefLinear(scip, intrasite.at(site*T + t), var, 1.0);
            }

            if (Param.masterSSBI) {


                ///RSU
                int l = inst->getl(i);
                int n = inst->getn();

                if ( i>0 ) {
                    int l_i_1=inst->getl(i-1);
                    if (t >= l_i_1 && (!inst->getLast(i-1)) ) {
                        SCIPaddCoefLinear(scip, rsu.at((i-1)*T + t), var, -1.0);
                    }
                }

                if ( i<n-1 && (!inst->getLast(i)) )  {

                    int t_min = fmax(t, l);
                    int t_max = fmin(T-1,t+l-1);
                    for (int k=t_min ; k <= t_max ; k++) {
                        SCIPaddCoefLinear(scip, rsu.at((i)*T + k), var, 1.0);
                    }
                }



                  ///RSD

                  if (!Param.RSUonly) {
                  int L = inst->getL(i);

                  if (i<n-1 && (!inst->getLast(i)) && t >= L ) {
                      SCIPaddCoefLinear(scip, rsd.at((i)*T + t), var, -1.0);
                  }

                  if ( i>0 && (!inst->getLast(i-1)) ) {

                      int t_min = fmax(t, L);
                      int t_max = fmin(T-1,t+L-1);
                      for (int k=t_min ; k <= t_max ; k++) {
                          SCIPaddCoefLinear(scip, rsd.at((i-1)*T + k), var, 1.0);
                      }
                  }
                  }

            }
        }
    }


    ///////////////////////////////////////////////////////////////
    //////////   MASTER LAMBDA VARIABLES INITIALIZATION   /////////
    ///////////////////////////////////////////////////////////////

    //Add variables corresponding to "all up" up/down plans (assuming initial state is up for all units), each unit produces at Pmax

    L_var.clear();

    int n = inst->getn() ;

    for (int t = 0 ; t<T ; t++)
    {
        IloNumArray plan = IloNumArray(env, n) ;
        IloNumArray powerplan = IloNumArray(env, n) ;
        for (int index=0 ; index < n ; index++) {
            plan[index]=1 ;
            powerplan[index]=inst->getPmax(index);
        }

        MasterTime_Variable* lambda = new MasterTime_Variable(t, plan);
        lambda->addPowerPlan(powerplan);
        initMasterTimeVariable(scip, lambda);

        SCIPaddVar(scip, lambda->ptr);


        addCoefsToConstraints(scip, lambda) ;

    }


    /// test

//        IloNumArray plan_test = IloNumArray(env, inst->nbUnits(0)*T) ;
//        for (int index=0 ; index < inst->nbUnits(0)*T ; index++) {
//            plan_test[index]=1 ;
//        }
//        plan_test[1*T] = 0 ;
//        plan_test[2*T] = 0 ;
//        plan_test[2*T+1] = 0 ;

//        Master_Variable* lambda = new Master_Variable(0, plan_test);
//        initMasterVariable(scip, inst, lambda);

//        SCIPaddVar(scip, lambda->ptr);

//        addCoefsToConstraints(scip, lambda, inst) ;


}



//////////////////////////////////////
////////// METHODES VIRTUELLES ///////
//////////////////////////////////////

void MasterTime_Model::computeFracSol(SCIP* scip) {
    list<MasterTime_Variable*>::const_iterator itv;
    SCIP_Real frac_value;
    for (int ind=0 ; ind < n*T ; ind++) {
        x_frac[ind]=0;
    }

    for (itv = L_var.begin(); itv!=L_var.end(); itv++) {

        frac_value = fabs(SCIPgetVarSol(scip,(*itv)->ptr));

        int time = (*itv)->time ;
        for (int i=0 ; i < n ; i++) {
            if ((*itv)->UpDown_plan[i] > 1 - Param.Epsilon) {
                x_frac[i*T+time] += frac_value ;
            }
        }
    }
}


void MasterTime_Model::discardVar(SCIP* scip, SCIP_ConsData* consdata) {

    /////On met à 0 les lambda incompatibles avec la contrainte

     consdata->L_var_bound.clear() ; // L_var_bound stocke les variables scip dont la borne a été effectivement changée (ie elle n'était pas déjà à 0)

     list<MasterTime_Variable*>::const_iterator itv;

     for (itv = L_var.begin(); itv!=L_var.end(); itv++) {
         if ((*itv)->time == consdata->time) {
             if ((*itv)->UpDown_plan[consdata->unit] != consdata->bound ) {

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

void MasterTime_Model::restoreVar(SCIP* scip, SCIP_ConsData* consdata) {

    ////On remet à +inf les lambda qui étaient incompatibles avec la contrainte de branchement

    list<SCIP_VAR*>::const_iterator itv;

    for (itv = consdata->L_var_bound.begin(); itv!=consdata->L_var_bound.end(); itv++) {
        SCIPchgVarUbNode(scip, NULL, (*itv), SCIPinfinity(scip)) ;
    }
    consdata->L_var_bound.clear() ;
}
