#include "Master.h"

/* namespace usage */
using namespace std;
using namespace scip;

MasterTime_Variable::MasterTime_Variable(int t, IloNumArray UpDown, double costFromSubPb) {
    ptr = NULL ;
    time = t ;
    UpDown_plan = UpDown  ;
    cost= costFromSubPb ;
}

void MasterTime_Model::addCoefsToConstraints(SCIP* scip, MasterTime_Variable* lambda, InstanceUCP* inst) {

    int t = lambda->time ;

    /* for each unit i, add coefficient 1 to logical constraint logical(i,t) if t >=1, add coefficient -1 to logical(i, t+1) if t < T-1*/

    for (int i=0 ; i < n ; i++) {
        if (lambda->UpDown_plan[i]) {
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
        if (lambda->UpDown_plan[i]) {
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

}


void MasterTime_Model::initMasterTimeVariable(SCIP* scip, InstanceUCP* inst , MasterTime_Variable* var) {

    char var_name[255];
    SCIPsnprintf(var_name, 255, "V_%d",L_var.size());
    SCIPdebugMsg(scip, "new variable <%s>\n", var_name);

    /* create the new variable: Use upper bound of infinity such that we do not have to care about
     * the reduced costs of the variable in the pricing. The upper bound of 1 is implicitly satisfied
     * due to the set partitioning constraints.
     */

    // Dans ce cas le coûts est initialisé dès la déclaration de la variable
    double cost= var->cost;
    cout << var_name << ", cost: " << cost << endl ;

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


    cout << "Variable " << var_name << " added, with plan:" << endl ;

    for (int i=0 ; i < n ; i++) {
        cout << var->UpDown_plan[i] << " " ;
    }

}

MasterTime_Model::MasterTime_Model(InstanceUCP* inst, const Parameters & Parametres) : Param(Parametres) {

    n = inst->getn() ;
    T = inst->getT() ;
    S = inst->getS() ;

    logical.resize(n*T, (SCIP_CONS*) NULL) ;
    min_up.resize(n*T, (SCIP_CONS*) NULL) ;
    min_down.resize(n*T, (SCIP_CONS*) NULL) ;

    convexity_cstr.resize(T, (SCIP_CONS*) NULL) ;

}

void  MasterTime_Model::InitScipMasterTimeModel(SCIP* scip, InstanceUCP* inst) {

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


    /////////////////////////////////////////////////
    ////////   MASTER START UP VARIABLES   //////////
    /////////////////////////////////////////////////

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
                          SCIP_VARTYPE_INTEGER, // variable type
                          true, false, NULL, NULL, NULL, NULL, NULL);


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
                SCIPaddCoefLinear(scip, min_down.at(i*T + k), var, -1.0);
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
        for (int index=0 ; index < n ; index++) {
            plan[index]=1 ;
        }

        double cost=0 ;
        for (int i=0 ; i <n ; i++) {
            cost += (inst->getcf(i) + inst->getPmax(i)*inst->getcp(i)) ;
        }

        MasterTime_Variable* lambda = new MasterTime_Variable(t, plan, cost);
        initMasterTimeVariable(scip, inst, lambda);

        SCIPaddVar(scip, lambda->ptr);

        addCoefsToConstraints(scip, lambda, inst) ;
    }




    /// test

    //    IloNumArray plan_test = IloNumArray(env, inst->nbUnits(0)*T) ;
    //    for (int index=0 ; index < inst->nbUnits(0)*T ; index++) {
    //        plan_test[index]=1 ;
    //    }
    //    plan_test[1*T] = 0 ;
    //    plan_test[2*T] = 0 ;
    //    plan_test[2*T+1] = 0 ;

    //    Master_Variable* lambda = new Master_Variable(0, plan_test);
    //    initMasterVariable(scip, inst, lambda);

    //    SCIPaddVar(scip, lambda->ptr);

    //    addCoefsToConstraints(scip, lambda, inst) ;

}
