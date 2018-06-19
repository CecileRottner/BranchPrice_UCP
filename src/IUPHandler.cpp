#include "IUPHandler.h"

#include "Pricer.h"
#include "Master.h"

#define eps 1e-6
#define OUTPUT_HANDLER


/////////////////////////////////////////////
SCIP_RETCODE IUPHandler::scip_check(
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

//    int T = inst->getT() ;
//    int n = inst->getn() ;

//    vector<double> x_frac = vector<double>(n*T, 0) ;

//    list<MasterTime_Variable*>::const_iterator itv;
//    SCIP_Real frac_value;

//    for (itv = Master->L_var.begin(); itv!=Master->L_var.end(); itv++) {

//        frac_value = fabs(SCIPgetVarSol(scip,(*itv)->ptr));

//    }

    *result = SCIP_FEASIBLE;
    return SCIP_OKAY;

}

SCIP_RETCODE IUPHandler::scip_enfolp(
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




SCIP_RETCODE IUPHandler::scip_trans(
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

//    sourcedata = SCIPconsGetData(sourcecons);
//    targetdata = NULL;

//    targetdata= new SCIP_CONSDATA;
//    targetdata->VarX = sourcedata->VarX;
//    targetdata->bound = sourcedata->bound;
//    targetdata->unit = sourcedata->unit;
//    targetdata->time = sourcedata->time;
//    targetdata->site = sourcedata->site;
//    targetdata->BranchConstraint = sourcedata->BranchConstraint;

    SCIPcreateCons(scip, targetcons, SCIPconsGetName(sourcecons), conshdlr, targetdata,
                   SCIPconsIsInitial(sourcecons), SCIPconsIsSeparated(sourcecons), SCIPconsIsEnforced(sourcecons),
                   SCIPconsIsChecked(sourcecons), SCIPconsIsPropagated(sourcecons),
                   SCIPconsIsLocal(sourcecons), SCIPconsIsModifiable(sourcecons),
                   SCIPconsIsDynamic(sourcecons), SCIPconsIsRemovable(sourcecons), SCIPconsIsStickingAtNode(sourcecons));



    return SCIP_OKAY;
}



SCIP_RETCODE IUPHandler::scip_enfops(
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

SCIP_RETCODE IUPHandler::scip_lock(
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

SCIP_RETCODE IUPHandler::scip_sepalp(
        SCIP*              scip,               /**< SCIP data structure */
        SCIP_CONSHDLR*     conshdlr,           /**< the constraint handler itself */
        SCIP_CONS**        conss,              /**< array of constraints to process */
        int                nconss,             /**< number of constraints to process */
        int                nusefulconss,       /**< number of useful (non-obsolete) constraints to process */
        SCIP_RESULT*       result) {

#ifdef OUTPUT_HANDLER
    std::cout << " --------------------- Sepalp IUP handler ---------------  \n";
#endif


    *result = SCIP_DIDNOTRUN;
    return SCIP_OKAY;
}

SCIP_RETCODE IUPHandler::scip_sepasol(SCIP* scip, SCIP_CONSHDLR* conshdlr, SCIP_CONS** conss,
                                             int nconss, int nusefulconss, SCIP_SOL* sol, SCIP_RESULT* result){

#ifdef OUTPUT_HANDLER
    std::cout << " --------------------- Sepasol handler ---------------  \n";
#endif

    *result = SCIP_DIDNOTRUN;
    return SCIP_OKAY;
}
