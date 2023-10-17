#include "CompactSCIP.h"

/* namespace usage */
using namespace std;
using namespace scip;



Compact_Model::Compact_Model(const Parameters & Parametres, InstanceUCP* inst) : Param(Parametres), inst(inst){
    n = inst->getn() ;
    T = inst->getT() ;

    demand_cstr.resize(T, (SCIP_CONS*) NULL);
    power_limits.resize(n*T, (SCIP_CONS*) NULL) ;
    min_down.resize(n*T, (SCIP_CONS*) NULL) ;
    min_up.resize(n*T, (SCIP_CONS*) NULL) ;
    logical.resize(n*T, (SCIP_CONS*) NULL) ;

    downtime.resize(n*T*T, (SCIP_CONS*) NULL) ;
    startup.resize(n*T, (SCIP_CONS*) NULL) ;

    x_var.resize(n*T, (SCIP_VAR*) NULL) ;
    u_var.resize(n*T, (SCIP_VAR*) NULL) ;
    p_var.resize(n*T, (SCIP_VAR*) NULL) ;
    u_temps_var.resize(n*T*T, (SCIP_VAR*) NULL) ;
}

void  Compact_Model::InitScipCompactModel(SCIP* scip, InstanceUCP* inst) {

    ///// Demand constraint /////
    char con_name_demand[255];
    for (int t = 0; t < T; t++)
    {
        SCIP_CONS* con = NULL;
        (void) SCIPsnprintf(con_name_demand, 255, "Demand(%d)", t); // nom de la contrainte
        SCIPcreateConsLinear( scip, &con, con_name_demand, 0, NULL, NULL,
                              inst->getD(t),   // lhs
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
            power_limits[i*T + t] = con;
        }
    }

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

    ///// Non-linear startup cost constraints /////
    if (Param.nonLinearStartUpCost){

        char con_name_downtime[255];
        char con_name_startup[255];
        for (int i = 0 ; i <n ; i++)
        {
            for (int t = 1; t < T; t++)
            {

                SCIP_CONS* con = NULL;
                (void) SCIPsnprintf(con_name_startup, 255, "Startup(%d,%d)", i, t); // nom de la contrainte
                SCIPcreateConsLinear( scip, &con, con_name_startup, 0, NULL, NULL,
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
                startup.at(i*T + t) = con;

                for (int down = 1; down < t + 1; down++){
                    
                    
                    if (t - down > 0){
                        SCIP_CONS* con = NULL;
                        (void) SCIPsnprintf(con_name_downtime, 255, "Downtime(%d,%d,%d)", i, t, down); // nom de la contrainte
                        SCIPcreateConsLinear( scip, &con, con_name_downtime, 0, NULL, NULL,
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
                        downtime.at(i*T*T + t*T + down) = con;
                    }
                }
            }
        }
    }

    


    ////////  VARIABLES INITIALIZATION   //////////
    char var_name[255];

    for (int i = 0 ; i <n ; i++)
    {
        for (int t = 0; t < T; t++)
        {
            //x variables

             SCIP_VAR* varx = NULL;

            SCIPsnprintf(var_name, 255, "x(%d,%d)",i,t);
            SCIPdebugMsg(scip, "Variable <%s>\n", var_name);

            SCIPcreateVar(scip, &varx, var_name,
                          0.0,                     // lower bound
                          1,      // upper bound
                          inst->getcf(i) + inst->getPmin(i)*inst->getcp(i),                     // objective
                          SCIP_VARTYPE_INTEGER, // variable type
                          true, false, NULL, NULL, NULL, NULL, NULL);

            SCIPaddVar(scip, varx);
            
            SCIPaddCoefLinear(scip, power_limits[i*T + t], varx, inst->getPmax(i) - inst->getPmin(i));

            SCIPaddCoefLinear(scip, demand_cstr[t], varx, inst->getPmin(i)) ;

            if (t>0) {
                SCIPaddCoefLinear(scip, logical.at(i*T+t), varx, 1.0) ;
            }
            if (t < T-1) {
                SCIPaddCoefLinear(scip, logical.at(i*T+t+1), varx, -1.0) ;
            }

            if (t>=inst->getL(i)) {
                SCIPaddCoefLinear(scip, min_up.at(i*T+t), varx, 1.0) ;
            }
            int l = inst->getl(i) ;
            if (t < T - l) {
                SCIPaddCoefLinear(scip, min_down.at(i*T+t+l), varx, 1.0) ;
            }

            if (Param.nonLinearStartUpCost){
                for (int down = 1; down < t - 1; down++){

                    SCIPaddCoefLinear(scip, downtime.at(i*T*T + t*T + down + 1), varx, 1.0);
                }
            }


            //u variables

            if (t>0){
                SCIP_VAR* varu = NULL;

                SCIPsnprintf(var_name, 255, "u(%d,%d)",i,t);
                SCIPdebugMsg(scip, "Variable <%s>\n", var_name);

                if (Param.nonLinearStartUpCost){
                    SCIPcreateVar(scip, &varu, var_name,
                                0.0,                     // lower bound
                                1,      // upper bound
                                0,                     // objective
                                SCIP_VARTYPE_INTEGER, // variable type
                                true, false, NULL, NULL, NULL, NULL, NULL);
                }
                else{
                    SCIPcreateVar(scip, &varu, var_name,
                                0.0,                     // lower bound
                                1,      // upper bound
                                inst->getc0(i),                     // objective
                                SCIP_VARTYPE_INTEGER, // variable type
                                true, false, NULL, NULL, NULL, NULL, NULL);
                }

                SCIPaddVar(scip, varu);
                
                SCIPaddCoefLinear(scip, logical.at(i*T+t), varu, -1.0);

                /* add coefficients to the min up constraints */
                int max_min_up = fmin(T-1, t + inst->getL(i) - 1) ;
                int min_min_up = fmax(inst->getL(i), t) ;
                // u(i,t) apparait dans les contraintes de min-up de min_min_up à max_min_up :
                for (int k = min_min_up ; k <= max_min_up ; k++) {
                    SCIPaddCoefLinear(scip, min_up.at(i*T + k), varu, -1.0);
                }

                /* add coefficients to the min down constraints */
                int max_min_down = fmin(T-1, t + inst->getl(i) - 1) ;
                int min_min_down = fmax(inst->getl(i), t);
                // u(i,t) apparait dans les contraintes de min-down de t à max_min_down :
                for (int k = min_min_down ; k <= max_min_down ; k++) {
                    SCIPaddCoefLinear(scip, min_down.at(i*T + k), varu, 1.0);
                }

                if (Param.nonLinearStartUpCost){
                    SCIPaddCoefLinear(scip, startup.at(i*T + t), varu, -1.0);
                }



                //variables specific to nonlinear startup costs
                if (Param.nonLinearStartUpCost){
                    
                    //u_temps
                    for (int down = 1; down < t + 1; down++){


                        SCIP_VAR* varut = NULL;

                        SCIPsnprintf(var_name, 255, "u_temps(%d,%d,%d)",i,t,down);
                        SCIPdebugMsg(scip, "Variable <%s>\n", var_name);

                        SCIPcreateVar(scip, &varut, var_name,
                                0.0,                     // lower bound
                                1,      // upper bound
                                (1 - exp(-float(down)/12)) * inst->getc0(i),                     // objective
                                SCIP_VARTYPE_INTEGER, // variable type
                                true, false, NULL, NULL, NULL, NULL, NULL);

                        SCIPaddVar(scip, varut);

                        SCIPaddCoefLinear(scip, startup.at(i*T + t), varut, 1.0);

                        if (t - down > 0){
                            
                            SCIPaddCoefLinear(scip, downtime.at(i*T*T + t*T + down), varut, -1.0);

                        }

                    }

                }
            }


            //p variables 

            SCIP_VAR* varp = NULL;

            SCIPsnprintf(var_name, 255, "p(%d,%d)",i,t);
            SCIPdebugMsg(scip, "Variable <%s>\n", var_name);


            SCIPcreateVar(scip, &varp, var_name,
                          0.0,                     // lower bound
                          inst->getPmax(i) - inst->getPmin(i),      // upper bound
                          inst->getcp(i),                     // objective
                          SCIP_VARTYPE_CONTINUOUS, // variable type
                          true, false, NULL, NULL, NULL, NULL, NULL);

            SCIPaddVar(scip, varp);

            /* add coefficient to the demand constraint */
            SCIPaddCoefLinear(scip, demand_cstr[t], varp, 1.0);

            /* add coefficient to the power limit constraint */
            SCIPaddCoefLinear(scip, power_limits[i*T + t], varp, -1.0);

        }
    }


}