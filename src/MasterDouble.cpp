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
                }
            }

            else {
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
    L_var_site.push_back(var);


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

    // Dans ce cas le coût est initialisé dès la déclaration de la variable
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

    cout << "Variable " << var_name << " added, with plan: " << endl  ;

    for (int i=0 ; i < inst->getn() ; i++) {
        cout << var->UpDown_plan[i] << " "  ;
    }
}

MasterDouble_Model::MasterDouble_Model(InstanceUCP* inst, const Parameters & Parametres) : Master_Model(Parametres, inst) {
    n = inst->getn() ;
    T = inst->getT() ;


    S = Parametres.nbDecGpes ;

    conv_lambda_site.resize(S, (SCIP_CONS*) NULL) ;
    conv_lambda_time.resize(T, (SCIP_CONS*) NULL) ;
    eq_time_site.resize(n*T, (SCIP_CONS*) NULL) ;

}

void  MasterDouble_Model::initScipMasterDoubleModel(SCIP* scip, InstanceUCP* inst) {


    ////////////////////////////////////////////////////////////////
    /////////////   MASTER CONSTRAINT INITIALIZATION   /////////////
    ////////////////////////////////////////////////////////////////
    // Constraints form: lhs <= ax <= rhs


    ///// Equality time/site constraint /////
    char con_name_eq_time_site[255];
    for (int i = 0 ; i <n ; i++)
    {
        for (int t = 0; t < T; t++)
        {
            SCIP_CONS* con = NULL;
            (void) SCIPsnprintf(con_name_eq_time_site, 255, "EqTimeSite(%d,%d)", i, t); // nom de la contrainte
            SCIPcreateConsLinear( scip, &con, con_name_eq_time_site, 0, NULL, NULL,
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




    ///////////////////////////////////////////////////////////////
    //////////   SITE LAMBDA VARIABLES INITIALIZATION   /////////
    ///////////////////////////////////////////////////////////////

    //Add variables corresponding to "all up" up/down plans (assuming initial state is up for all units)

    L_var_site.clear();


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


        initMasterSiteVariable(scip, inst, lambda);
        SCIPaddVar(scip, lambda->ptr);
        addCoefsToConstraints_siteVar(scip, lambda, inst) ;
    }



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

        double cost=0 ;
        for (int i=0 ; i <n ; i++) {
            cost += (inst->getPmax(i) - inst->getPmin(i))*inst->getcp(i) ;
        }

        MasterTime_Variable* lambda = new MasterTime_Variable(t, plan, cost);
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

//    consdata->L_var_bound.clear() ; // L_var_bound stocke les variables scip dont la borne a été effectivement changée (ie elle n'était pas déjà à 0)

//    list<Master_Variable*>::const_iterator itv;

//    for (itv = L_var.begin(); itv!=L_var.end(); itv++) {
//        if ((*itv)->Site == consdata->site) {
//            if ((*itv)->UpDown_plan[consdata->unit*T + consdata->time] != consdata->bound ) {

//                SCIP_Real old_bound =  SCIPgetVarUbAtIndex(scip, (*itv)->ptr, NULL, 0) ;

//                ///  L_var_bound est mis à jour
//                if (!SCIPisZero(scip,old_bound)) {
//                    SCIPchgVarUbNode(scip, NULL, (*itv)->ptr, 0) ;
//                    consdata->L_var_bound.push_back((*itv)->ptr) ;
//                }
//            }
//        }
//    }
}

void MasterDouble_Model::restoreVar(SCIP* scip, SCIP_ConsData* consdata) {

//    ////On remet à +inf les lambda qui étaient incompatibles avec la contrainte de branchement

//    list<SCIP_VAR*>::const_iterator itv;

//    for (itv = consdata->L_var_bound.begin(); itv!=consdata->L_var_bound.end(); itv++) {
//        SCIPchgVarUbNode(scip, NULL, (*itv), SCIPinfinity(scip)) ;
//    }
//    consdata->L_var_bound.clear() ;

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

        double cost = 0 ;

        MasterTime_Variable* lambda = new MasterTime_Variable(t, plan, cost) ;
        initMasterTimeVariable(scip, lambda);

        SCIPaddVar(scip, lambda->ptr);

        addCoefsToConstraints_timeVar(scip, lambda) ;
    }
}
