#include "IUPHandler.h"

#include "Pricer.h"
#include "Master.h"
#include "IntervalUpSet.h"
#include "Process.h"

#define eps 1e-6
#define OUTPUT_HANDLER


IUPHandler::IUPHandler(SCIP* scip, MasterTime_Model* M, InstanceUCP* i, const Parameters & Pa) :
    scip::ObjConshdlr(
        scip,
        "IUPHandler",                    // const char *  	name,
        "Handler For Interval Up Set inequalities",   // const char *  	desc,
        2000000, -2000000, -2000000,           // int sepapriority, int enfopriority, int checkpriority,
        1, -1, 1, 0,                           // int sepafreq, int propfreq, int eagerfreq, int maxprerounds,
        FALSE, FALSE, FALSE,                   // delaysepa, delayprop, needscons,
        SCIP_PROPTIMING_BEFORELP,              // SCIP_PROPTIMING  	proptiming,
        SCIP_PRESOLTIMING_FAST                 // SCIP_PRESOLTIMING  	presoltiming
        ),
    Param(Pa)

{
    Master = M ;
    inst = i ;
    int n= inst->getn() ;
    int T = inst->getT() ;
    x_frac = vector<double>(n*T, 0) ;
    u_frac = vector<double>(n*T, 0) ;
    Sep = new Separation(inst, Param.Epsilon) ;

    nbFound=0 ;
}



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
    std::cout << " --------------------- Check IUP handler ---------------  \n";
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
    std::cout << " --------------------- Lock IUP handler ---------------  \n";
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

    if (Master->Relax_withoutIUP == 0) {
        Master->Relax_withoutIUP = SCIPgetDualbound(scip);
    }

    int n = inst->getn() ;
    int T = inst->getT() ;
    for (int ind = 0 ; ind < n*T ; ind++) {
        x_frac[ind]=0 ;
        //u_frac[ind]=0 ;
    }

    *result = SCIP_DIDNOTRUN;

    /// récupération solution fractionnaire ////
    list<MasterTime_Variable*>::const_iterator itv;
    SCIP_Real frac_value;

    for (itv = Master->L_var.begin(); itv!=Master->L_var.end(); itv++) {

        frac_value = fabs(SCIPgetVarSol(scip,(*itv)->ptr));

        int time = (*itv)->time ;
        for (int i=0 ; i < n ; i++) {

            if ((*itv)->UpDown_plan[i] > 1 - Param.Epsilon) {
                x_frac[i*T+time] += frac_value ;
            }
        }
    }

    for (int t=1 ; t < T ; t++) {
        for (int i=0 ; i <n ; i++) {
            u_frac[i*T+t] = fabs(SCIPgetVarSol(scip,Master->u_var[i*T+t]));
        }
    }
//    cout << "solution x frac: " << endl;

//    for (int t=0 ; t < T ; t++) {
//        for (int i=0 ; i <n ; i++) {
//            cout << x_frac[i*T+t] << " " ;
//        }
//        cout << endl ;
//    }
    //// Separation ////

    for (int t0=0 ; (t0 < T) ; t0++) {
        for (int t1 = t0+1 ; (t1 <= fmin(T-1, t0+Sep->Lmax)) ; t1++) {

            //Mise à jour des coûts
            Sep->computeCosts(t0, t1, x_frac, u_frac) ;
            Sep->computeWeightedCosts(t0, t1, x_frac, u_frac) ;

            for (int i0 = 0 ; (i0 < n) ; i0++) {
                list<int>* C_list = new list<int> ;
                C_list->clear() ;
                if (Sep->iOK(i0, t0, t1)) {
                    double alpha = Sep->SepareSCIP(C_list, i0, t0, t1) ;
                    if (alpha > 0) {
                        int i =  inst->getTri(i0) ;

//                        // Check it is violated
//                        double violation = - alpha ;


//                        for (int t=t0+1 ; t <= t1 ; t++) {
//                            violation -= fabs(SCIPgetVarSol(scip,Master->u_var[i*T+t]));
//                            list<int>::const_iterator j;


//                        }

                        IneqIntUpSet* ineq = new IneqIntUpSet(scip, nbFound, alpha, C_list, i, t0, t1) ;
                        Master->addIntUpSet(scip,ineq);

                        *result = SCIP_CONSADDED;
                        nbFound++ ;
                    }
                }
            }
        }
    }
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
