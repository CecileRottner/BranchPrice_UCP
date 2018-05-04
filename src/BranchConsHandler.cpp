#include"BranchConsHandler.h"

#include "Pricer.h"
#include "Master.h"

#define eps 1e-6
#define OUTPUT_HANDLER

//////////////////////////////////////////////
//////////////////////////////////////////////
void createBranchCstr(SCIP* scip, int VarX, int bound, int unit, int time, int site, ObjPricerUCP* pricer, SCIP_CONS** cons) {


#ifdef OUTPUT_HANDLER
    cout << " ------ CREATE A CONSTRAINT ASSOCIATED TO A NODE   ---------------  \n";
#endif


    // initialise les donnees specifiques au noeud fils
    SCIP_ConsData* consdata = new SCIP_ConsData;
    SCIP_CONSHDLR* conshdlr = SCIPfindConshdlr(scip, "BranchConsHandler");

#ifdef OUTPUT_HANDLER
    if (conshdlr==NULL) cout<<"CONSTRAINT HANDLER NOT FOUND -> CHECK SCIP_DEBUG TO SEE ITS PARAMETERS"<<endl;
#endif

    // Création des données liées à la contrainte de branchement
    consdata->VarX = VarX;
    consdata->bound = bound;
    consdata->unit = unit ;
    consdata->time = time ;
    consdata->site = site ;

    int T = pricer->inst->getT() ;

    if (VarX) {
        consdata->BranchConstraint = ((pricer->AlgoCplex[site])->x[unit*T+time] == bound) ;
    }
    else {
        consdata->BranchConstraint = ((pricer->AlgoCplex[site])->u[unit*T+time] == bound) ;
    }


    SCIPcreateCons(scip, cons, "BranchConsCstr", conshdlr, consdata,
                   FALSE, //initial
                   FALSE, //separate
                   FALSE, //enforce
                   FALSE, //check
                   TRUE,  //propagate
                   TRUE,  //local
                   FALSE, //modifiable
                   FALSE, //dynamic
                   FALSE, //removable
                   TRUE); //stickinganode



#ifdef OUTPUT_HANDLER
    cout << " ------ END CREATION  ---------------  \n";
#endif

}


//////////////////////////////////////////////
//////////////////////////////////////////////
SCIP_RETCODE BranchConsHandler::scip_active(SCIP * scip, SCIP_CONSHDLR * conshdlr, SCIP_CONS * cons) {
#ifdef OUTPUT_HANDLER
    cout << " --------------------- Active handler ---------------  \n";
#endif

    int T = inst->getT() ;

    SCIP_ConsData *consdata = SCIPconsGetData(cons);

    cout << "Active node: unit " << consdata->unit << ", time " << consdata->time << " at bound " << consdata->bound<< endl;

    //On ajoute la contrainte dans cons au modèle Cplex du sous problème correspondant
    pricer_ptr->AlgoCplex[consdata->site]->model.add(consdata->BranchConstraint) ;

    //On met à 0 les lambda incompatibles avec la contrainte
    consdata->L_var_bound.clear() ;

    list<Master_Variable*>::const_iterator itv;

    for (itv = Master->L_var.begin(); itv!=Master->L_var.end(); itv++) {
        if ((*itv)->Site == consdata->site) {
            if ((*itv)->UpDown_plan[consdata->unit*T + consdata->time] != consdata->bound ) {

                SCIP_Real old_bound =  SCIPgetVarUbAtIndex(scip, (*itv)->ptr, NULL, 0) ;
               // cout << "variable " << SCIPvarGetName((*itv)->ptr) << ", old bound: " << old_bound << endl ;
                if (!SCIPisZero(scip,old_bound)) {
                    SCIPchgVarUbNode(scip, NULL, (*itv)->ptr, 0) ;
                    consdata->L_var_bound.push_back((*itv)) ;
                }
            }
        }
    }


#ifdef OUTPUT_HANDLER
    cout << " --------------------- Fin Active handler ---------------  \n";
#endif


    return SCIP_OKAY;
}

//////////////////////////////////////////////
//////////////////////////////////////////////
SCIP_RETCODE BranchConsHandler::scip_deactive(SCIP* scip, SCIP_CONSHDLR* conshdlr, SCIP_CONS*cons){
#ifdef OUTPUT_HANDLER
    cout << " --------------------- Desactive handler ---------------  \n";
#endif


    SCIP_ConsData *consdata = SCIPconsGetData(cons);

    cout << "Deactive node: unit " << consdata->unit << ", time " << consdata->time << " at bound " << consdata->bound<< endl;
    //On retire la contrainte dans cons au modèle Cplex du sous problème correspondant
    pricer_ptr->AlgoCplex[consdata->site]->model.remove(consdata->BranchConstraint) ;


    //On remet à +inf les lambda qui étaient incompatibles avec la contrainte de branchement
    int T = inst->getT() ;
    list<Master_Variable*>::const_iterator itv;

    for (itv = consdata->L_var_bound.begin(); itv!=consdata->L_var_bound.end(); itv++) {
        if ((*itv)->Site == consdata->site) {
            if ((*itv)->UpDown_plan[consdata->unit*T + consdata->time] != consdata->bound ) {
                //cout << "variable " << SCIPvarGetName((*itv)->ptr) << ": bound a l'infini" << endl ;
                SCIPchgVarUbNode(scip, NULL, (*itv)->ptr, SCIPinfinity(scip)) ;
            }
        }
    }
    consdata->L_var_bound.clear() ;

    return SCIP_OKAY;
}


//////////////////////////////////////////////
/** transforms constraint data into data belonging to the transformed problem */
SCIP_RETCODE BranchConsHandler::scip_trans(
        SCIP*              scip,               //**< SCIP data structure *
        SCIP_CONSHDLR*     conshdlr,           //**< the constraint handler itself *
        SCIP_CONS*         sourcecons,         //**< source constraint to transform *
        SCIP_CONS**        targetcons          //**< pointer to store created target constraint *
        ) {

#ifdef OUTPUT_HANDLER
    std::cout << " --------------------- Trans handler ---------------  \n";
#endif

    SCIP_CONSDATA* sourcedata;
    SCIP_CONSDATA* targetdata;

    sourcedata = SCIPconsGetData(sourcecons);
    targetdata = NULL;

    targetdata= new SCIP_CONSDATA;
    targetdata->VarX = sourcedata->VarX;
    targetdata->bound = sourcedata->bound;
    targetdata->unit = sourcedata->unit;
    targetdata->time = sourcedata->time;
    targetdata->site = sourcedata->site;
    targetdata->BranchConstraint = sourcedata->BranchConstraint;

    SCIPcreateCons(scip, targetcons, SCIPconsGetName(sourcecons), conshdlr, targetdata,
                   SCIPconsIsInitial(sourcecons), SCIPconsIsSeparated(sourcecons), SCIPconsIsEnforced(sourcecons),
                   SCIPconsIsChecked(sourcecons), SCIPconsIsPropagated(sourcecons),
                   SCIPconsIsLocal(sourcecons), SCIPconsIsModifiable(sourcecons),
                   SCIPconsIsDynamic(sourcecons), SCIPconsIsRemovable(sourcecons), SCIPconsIsStickingAtNode(sourcecons));



    return SCIP_OKAY;
}


/////////////////////////////////////////////
SCIP_RETCODE BranchConsHandler::scip_check(
        SCIP*              scip,               /**< SCIP data structure */
        SCIP_CONSHDLR*     conshdlr,           /**< the constraint handler itself */
        SCIP_CONS**        conss,              /**< array of constraints to process */
        int                nconss,             /**< number of constraints to process */
        SCIP_SOL*          sol,                /**< the solution to check feasibility for */
        SCIP_Bool          checkintegrality,   /**< has integrality to be checked? */
        SCIP_Bool          checklprows,        /**< have current LP rows to be checked? */
        SCIP_Bool          printreason,        /**< should the reason for the violation be printed? */
        SCIP_Bool          completely,         /**< should all violations be checked? */
        SCIP_RESULT*       result) {

#ifdef OUTPUT_HANDLER
    std::cout << " --------------------- Check handler ---------------  \n";
#endif

    //cout << "solution du PMR:" << endl ;
    //SCIPprintSol(scip, NULL, NULL, FALSE);

    // Search for fractional x variables

    int T = pricer_ptr->inst->getT() ;
    int n = pricer_ptr->inst->getn() ;

    vector<double> x_frac = vector<double>(n*T, 0) ;

    list<Master_Variable*>::const_iterator itv;
    SCIP_Real frac_value;

    for (itv = Master->L_var.begin(); itv!=Master->L_var.end(); itv++) {

        frac_value = fabs(SCIPgetVarSol(scip,(*itv)->ptr));

        int site = (*itv)->Site ;
        int first = pricer_ptr->inst->firstUnit(site) ;
        for (int i=0 ; i < pricer_ptr->inst->nbUnits(site) ; i++) {
            for (int t=0 ; t < T ; t++) {
                if ((*itv)->UpDown_plan[i*T+t] > eps) {
                    x_frac[(first+i)*T+t] += frac_value ;
                }
            }
        }
    }

    for (int i=0 ; i < n ; i++) {
        for (int t=0 ; t < T ; t++) {
            if ( (x_frac[i*T+t] < 1-eps) && (x_frac[i*T+t] > eps) ) {
                cout << "solution fractionnaire" << endl;
                *result=SCIP_INFEASIBLE;
                return SCIP_OKAY;
            }
        }
    }

    *result = SCIP_FEASIBLE;
    return SCIP_OKAY;

}

SCIP_RETCODE BranchConsHandler::scip_enfolp(
        SCIP*              scip,               /**< SCIP data structure */
        SCIP_CONSHDLR*     conshdlr,           /**< the constraint handler itself */
        SCIP_CONS**        conss,              /**< array of constraints to process */
        int                nconss,             /**< number of constraints to process */
        int                nusefulconss,       /**< number of useful (non-obsolete) constraints to process */
        SCIP_Bool          solinfeasible,      /**< was the solution already declared infeasible by a constraint handler? */
        SCIP_RESULT*       result) {

#ifdef OUTPUT_HANDLER
    std::cout << " --------------------- Enfolp handler ---------------  \n";
#endif


    *result = SCIP_FEASIBLE;
    return SCIP_OKAY;
}

SCIP_RETCODE BranchConsHandler::scip_enfops(
        SCIP*              scip,               /**< SCIP data structure */
        SCIP_CONSHDLR*     conshdlr,           /**< the constraint handler itself */
        SCIP_CONS**        conss,              /**< array of constraints to process */
        int                nconss,             /**< number of constraints to process */
        int                nusefulconss,       /**< number of useful (non-obsolete) constraints to process */
        SCIP_Bool          solinfeasible,      /**< was the solution already declared infeasible by a constraint handler? */
        SCIP_Bool          objinfeasible,      /**< is the solution infeasible anyway due to violating lower objective bound? */
        SCIP_RESULT*       result) {

#ifdef OUTPUT_HANDLER
    std::cout << " --------------------- Enfops handler ---------------  \n";
#endif


    *result = SCIP_FEASIBLE;
    return SCIP_OKAY;
}

SCIP_RETCODE BranchConsHandler::scip_lock(
        SCIP*              scip,               /**< SCIP data structure */
        SCIP_CONSHDLR*     conshdlr,           /**< the constraint handler itself */
        SCIP_CONS*         cons,               /**< the constraint that should lock rounding of its variables, or NULL if the
                                                        *   constraint handler does not need constraints */
        int                nlockspos,          /**< no. of times, the roundings should be locked for the constraint */
        int                nlocksneg) {

#ifdef OUTPUT_HANDLER
    std::cout << " --------------------- Lock handler ---------------  \n";
#endif


    return SCIP_OKAY;
}

SCIP_RETCODE BranchConsHandler::scip_sepalp(
        SCIP*              scip,               /**< SCIP data structure */
        SCIP_CONSHDLR*     conshdlr,           /**< the constraint handler itself */
        SCIP_CONS**        conss,              /**< array of constraints to process */
        int                nconss,             /**< number of constraints to process */
        int                nusefulconss,       /**< number of useful (non-obsolete) constraints to process */
        SCIP_RESULT*       result) {

#ifdef OUTPUT_HANDLER
    std::cout << " --------------------- Sepalp handler ---------------  \n";
#endif


    *result = SCIP_DIDNOTRUN;
    return SCIP_OKAY;
}

SCIP_RETCODE BranchConsHandler::scip_sepasol(SCIP* scip, SCIP_CONSHDLR* conshdlr, SCIP_CONS** conss,
                                             int nconss, int nusefulconss, SCIP_SOL* sol, SCIP_RESULT* result){

#ifdef OUTPUT_HANDLER
    std::cout << " --------------------- Sepasol handler ---------------  \n";
#endif

    *result = SCIP_DIDNOTRUN;
    return SCIP_OKAY;
}
