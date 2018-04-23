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

void Master_Variable::computeCost(InstanceUCP* inst) {
    //compute cost of up/down plan lambda: fixed cost (including minimum power output cost) and start up cost
    //init à prendre en compte plus tard
    cost=0 ;

    int T= inst->getT() ;

    int first = inst->firstUnit(Site) ;
    int last = inst->firstUnit(Site) + inst->nbUnits(Site) - 1 ;


    // cout << "site " << Site << ", first unit : " << first << ", nb units: " << inst->nbUnits(Site) << endl ;

    for (int i = first ; i <= last ; i++) {
        int down_t_1 = 0 ;
        for (int t = 0 ; t < T ; t++) {
            if (down_t_1 && UpDown_plan[(i-first)*T+t]) { // il y a un démarrage en t
                cost+= inst->getc0(i) ;
            }

            if (UpDown_plan[(i-first)*T+t] == 0) {
                down_t_1=1;
            }
            else { // l'unité i est up en t
                cost+= inst->getcf(i) + inst->getcp(i)*inst->getPmin(i) ;
                down_t_1=0;
            }
        }
    }
}


void Master_Model::addCoefsToConstraints(SCIP* scip, Master_Variable* lambda, InstanceUCP* inst) {

    int s = lambda->Site ;
    int first = inst->firstUnit(s) ;

    /* for each time period, add coefficient pmin into the demand constraint at t, for each unit unit i up in site S */
    for (int t=0 ; t < T ; t++) {
        for (int i=0 ; i < inst->nbUnits(s) ; i++) {
            if (lambda->UpDown_plan[i*T+t]) {
                SCIPaddCoefLinear(scip, demand_cstr[t], lambda->ptr, inst->getPmin(first+i)) ;
            }
        }
    }

    /* for each time period and each unit in site S, add coefficient pmin(i) - pmax(i) into the power limit constraint of unit i at t */
    for (int t=0 ; t < T ; t++) {
        for (int i=0 ; i < inst->nbUnits(s) ; i++) {
            if (lambda->UpDown_plan[i*T+t]) {
                SCIPaddCoefLinear(scip, power_limits[(first+i)*T+t], lambda->ptr, inst->getPmax(first+i) - inst->getPmin(first+i)) ;
            }
        }
    }

    /* add coefficient to the convexity constraint for site s */
    SCIPaddCoefLinear(scip, convexity_cstr[s], lambda->ptr, 1.0) ;

}


Master_Model::Master_Model(InstanceUCP* inst) {
    n = inst->getn() ;
    T = inst->getT() ;
    S = inst->getS() ;

    demand_cstr.resize(T, (SCIP_CONS*) NULL);
    power_limits.resize(n*T, (SCIP_CONS*) NULL) ;
    convexity_cstr.resize(S, (SCIP_CONS*) NULL) ;

}

void  Master_Model::InitScipMasterModel(SCIP* scip, InstanceUCP* inst) {

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
                      1000,      // upper bound
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
            SCIPaddVar(scip, var);

            /* add coefficient to the demand constraint */
            SCIPaddCoefLinear(scip, demand_cstr[t], var, 1.0);

            /* add coefficient to the power limit constraint */
            SCIPaddCoefLinear(scip, power_limits[i*T + t], var, -1.0);
        }
    }

    ///////////////////////////////////////////////////////////////
    //////////   MASTER LAMBDA VARIABLES INITIALIZATION   /////////
    ///////////////////////////////////////////////////////////////

    //Add variables corresponding to "all up" up/down plans (assuming initial state is up for all units)

    L_var.clear();

    char varlambda_name[255];

    for (int s=0 ; s<S ; s++)
    {
        SCIPsnprintf(varlambda_name, 255, "all_up_(site_%d)",s);
        SCIPdebugMsg(scip, "new variable <%s>\n", varlambda_name);

        /* create the new variable: Use upper bound of infinity such that we do not have to care about
       * the reduced costs of the variable in the pricing. The upper bound of 1 is implicitly satisfied
       * due to the set partitioning constraints.
       */

        IloNumArray plan = IloNumArray(env, inst->nbUnits(s)*T) ;
        for (int index=0 ; index < inst->nbUnits(s)*T ; index++) {
            plan[index]=1 ;
        }

        Master_Variable* lambda = new Master_Variable(s, plan);

        lambda->computeCost(inst);
        double cost= lambda->cost;
        cout << "cost: " << cost << endl ;

        L_var.push_back(lambda);

        SCIPcreateVar(scip, &(lambda->ptr), varlambda_name,
                      0.0,                     // lower bound
                      SCIPinfinity(scip),      // upper bound
                      cost,                     // objective
                      SCIP_VARTYPE_INTEGER,    // variable type
                      true, false, NULL, NULL, NULL, NULL, NULL);


        /* add new variable to the list of variables to price into LP (score: leave 1 here) */
        SCIPaddVar(scip, lambda->ptr);

        addCoefsToConstraints(scip, lambda, inst) ;
    }

}
