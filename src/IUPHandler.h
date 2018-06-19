#ifndef IUPHandlerH
#define IUPHandlerH

#include "scip/scip.h"
#include "scip/cons_linear.h"

#include <ilcplex/ilocplex.h>

#include <scip/scipdefplugins.h>
#include "objscip/objscip.h"
#include "scip/cons_linear.h"

#include "Master.h"
#include "Pricer.h"
#include "IntervalUpSet.h"
#include "Process.h"

using namespace std;


#define SCIP_DEBUG



///////////////////////////:::

class IUPHandler : public scip::ObjConshdlr {

public :

    const Parameters Param ;
    MasterTime_Model* Master ;
    InstanceUCP* inst ;
    vector<double> x_frac ;
    vector<double> u_frac ;
    Separation* Sep ;

    IUPHandler(SCIP* scip, MasterTime_Model* M, InstanceUCP* i, const Parameters & Pa) ;

    //////////////////////////:
    //////////////////////////

    /** transforms constraint data into data belonging to the transformed problem */
    virtual SCIP_RETCODE scip_trans(
            SCIP*              scip,               //**< SCIP data structure *
            SCIP_CONSHDLR*     conshdlr,           //**< the constraint handler itself *
            SCIP_CONS*         sourcecons,         //**< source constraint to transform *
            SCIP_CONS**        targetcons          //**< pointer to store created target constraint *
            );


    virtual SCIP_RETCODE scip_check(
            SCIP*              scip,               /**< SCIP data structure */
            SCIP_CONSHDLR*     conshdlr,           /**< the constraint handler itself */
            SCIP_CONS**        conss,              /**< array of constraints to process */
            int                nconss,             /**< number of constraints to process */
            SCIP_SOL*          sol,                /**< the solution to check feasibility for */
            SCIP_Bool          checkintegrality,   /**< has integrality to be checked? */
            SCIP_Bool          checklprows,        /**< have current LP rows to be checked? */
            SCIP_Bool          printreason,        /**< should the reason for the violation be printed? */
            SCIP_Bool          completely,         /**< should all violations be checked? */
            SCIP_RESULT*       result              /**< pointer to store the result of the feasibility checking call */
            );

    virtual SCIP_RETCODE scip_enfolp(
            SCIP*              scip,               /**< SCIP data structure */
            SCIP_CONSHDLR*     conshdlr,           /**< the constraint handler itself */
            SCIP_CONS**        conss,              /**< array of constraints to process */
            int                nconss,             /**< number of constraints to process */
            int                nusefulconss,       /**< number of useful (non-obsolete) constraints to process */
            SCIP_Bool          solinfeasible,      /**< was the solution already declared infeasible by a constraint handler? */
            SCIP_RESULT*       result              /**< pointer to store the result of the enforcing call */
            );


    virtual SCIP_RETCODE scip_enfops(
            SCIP*              scip,               /**< SCIP data structure */
            SCIP_CONSHDLR*     conshdlr,           /**< the constraint handler itself */
            SCIP_CONS**        conss,              /**< array of constraints to process */
            int                nconss,             /**< number of constraints to process */
            int                nusefulconss,       /**< number of useful (non-obsolete) constraints to process */
            SCIP_Bool          solinfeasible,      /**< was the solution already declared infeasible by a constraint handler? */
            SCIP_Bool          objinfeasible,      /**< is the solution infeasible anyway due to violating lower objective bound? */
            SCIP_RESULT*       result              /**< pointer to store the result of the enforcing call */
            );

    virtual SCIP_RETCODE scip_lock(
            SCIP*              scip,               /**< SCIP data structure */
            SCIP_CONSHDLR*     conshdlr,           /**< the constraint handler itself */
            SCIP_CONS*         cons,               /**< the constraint that should lock rounding of its variables, or NULL if the
                                *   constraint handler does not need constraints */
            int                nlockspos,          /**< no. of times, the roundings should be locked for the constraint */
            int                nlocksneg           /**< no. of times, the roundings should be locked for the constraint's negation */
            );

    virtual SCIP_RETCODE scip_sepalp(
            SCIP*              scip,               /**< SCIP data structure */
            SCIP_CONSHDLR*     conshdlr,           /**< the constraint handler itself */
            SCIP_CONS**        conss,              /**< array of constraints to process */
            int                nconss,             /**< number of constraints to process */
            int                nusefulconss,       /**< number of useful (non-obsolete) constraints to process */
            SCIP_RESULT*       result              /**< pointer to store the result of the separation call */
            );

    virtual SCIP_RETCODE scip_sepasol(
            SCIP*              scip,               /**< SCIP data structure */
            SCIP_CONSHDLR*     conshdlr,           /**< the constraint handler itself */
            SCIP_CONS**        conss,              /**< array of constraints to process */
            int                nconss,             /**< number of constraints to process */
            int                nusefulconss,       /**< number of useful (non-obsolete) constraints to process */
            SCIP_SOL*          sol,                /**< primal solution that should be separated */
            SCIP_RESULT*       result              /**< pointer to store the result of the separation call */
            );
};




#endif
