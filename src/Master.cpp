#include "Master.h"

/* namespace usage */
using namespace std;
using namespace scip;

Master_Model::Master_Model(const InstanceUCP & inst) {
    n = inst.getn() ;
    T = inst.getT() ;
    S = inst.getS() ;

    demand_cstr.resize(T, (SCIP_CONS*) NULL);
    power_limits.resize(n*T, (SCIP_CONS*) NULL) ;
    convexity_cstr.resize(S, (SCIP_CONS*) NULL) ;

}

void  Master_Model::InitScipMasterModel(SCIP* scip, const InstanceUCP & inst) {

    ////////////////////////////////////////////////////////////////
    /////////////   MASTER CONSTRAINT INITIALIZATION   /////////////
    ////////////////////////////////////////////////////////////////
    // Constraints form: lhs <= ax <= rhs

    ///// Demand constraint ////
    char con_name_demand[255];
    for (int t = 0; t < T; t++)
    {
        SCIP_CONS* con = NULL;
        (void) SCIPsnprintf(con_name_demand, 255, "Demand(%d)", t); // nom de la contrainte
        SCIPcreateConsLinear( scip, &con, con_name_demand, 0, NULL, NULL,
                              inst.getD(t),   // lhs
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
        convexity_cstr[s] = con;
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
                          inst.getPmax(i) - inst.getPmin(i),      // upper bound
                          inst.getcp(i),                     // objective
                          SCIP_VARTYPE_CONTINUOUS, // variable type
                          true, false, NULL, NULL, NULL, NULL, NULL);


            /* add new variable to scip */
            SCIPaddVar(scip, var);

            /* add coefficient to the demand constraint */
            SCIPaddCoefLinear(scip, demand_cstr[t], var, 1.0);

            /* add coefficient to the power limit constraint */
            SCIPaddCoefLinear(scip, power_limits[i*T + t], var, 1.0);
        }
    }

    ///////////////////////////////////////////////////////////////
    //////////   MASTER LAMBDA VARIABLES INITIALIZATION   /////////
    ///////////////////////////////////////////////////////////////

    //Add variables corresponding to "all up" production plans (assuming initial state is up for all units)

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


      Master_Variable* lambda = new Master_Variable;
      lambda->Site = s ;
      lambda->Production_plan.resize(inst.nb(s)*T, 1);

      SCIPcreateVar(scip, &(lambda->ptr), varlambda_name,
                    0.0,                     // lower bound
                    SCIPinfinity(scip),      // upper bound
                    1.0,                     // objective
                    SCIP_VARTYPE_INTEGER, // variable type
                    true, false, NULL, NULL, NULL, NULL, NULL);


      /* add new variable to the list of variables to price into LP (score: leave 1 here) */
      SCIPaddVar(scip, lambda->ptr);

      /* add coefficient into the set partition constraints */
     // SCIPaddCoefLinear(scip, demand_cstr[t], lambda->ptr, 1.0);

      L_var.push_back(lambda);


    }


}
