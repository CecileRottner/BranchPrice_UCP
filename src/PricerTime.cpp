#include "Pricer.h"
#include "scip/cons_linear.h"
#include <map>
#include <vector>
#include <iostream>


#define OUTPUT_PRICER
// à décommenter pour l'affichage de debug

using namespace std;
using namespace scip;


/** Constructs the pricer object with the data needed
 *
 *  An alternative is to have a problem data class which allows to access the data.
 */
ObjPricerTimeUCP::ObjPricerTimeUCP(
        SCIP*                                scip,          /**< SCIP pointer */
        const char*                         pp_name,      /**< name of pricer */
        MasterTime_Model*                        M,
        InstanceUCP*                        instance,
        const Parameters &                  param
        ):
    ObjPricer(scip, pp_name, "Find production plans with negative reduced costs for time step.", 0, TRUE), Param(param)
{
    inst=instance ;
    Master=M;

    AlgoCplex = vector<CplexPricingAlgoTime*>(inst->getT(), NULL) ;

    for (int t=0 ; t < inst->getT() ; t++) {
        AlgoCplex[t] = new CplexPricingAlgoTime(inst, t) ;
    }
}


/** Destructs the pricer object. */
ObjPricerTimeUCP::~ObjPricerTimeUCP()
{
    cout<<"Destructeur du pricer"<<endl;
}

/** initialization method of variable pricer (called after problem was transformed)
 *
 *  Because SCIP transformes the original problem in preprocessing, we need to get the references to
 *  the variables and constraints in the transformed problem from the references in the original
 *  problem.
 */
SCIP_DECL_PRICERINIT(ObjPricerTimeUCP::scip_init)
{

    int T = inst->getT() ;
    int n = inst->getn();

    // logical constraints
    for (int i = 0 ; i <n ; i++) {
        for (int t = 1 ; t < T ; t++) {

        SCIPgetTransformedCons( scip, Master->logical.at(i*T+t), &(Master->logical.at(i*T+t)) );
        }
    }

    // min-up constraints
    for (int i = 0 ; i <n ; i++) {
        int L = inst->getL(i) ;
        for (int t = L ; t < T ; t++) {
            SCIPgetTransformedCons( scip, Master->min_up.at(i*T+t), &(Master->min_up.at(i*T+t)) );
        }
    }

    // min-down constraints
    for (int i = 0 ; i <n ; i++) {
        int l = inst->getl(i) ;
        for (int t = l ; t < T ; t++) {
            SCIPgetTransformedCons( scip, Master->min_down.at(i*T+t), &(Master->min_down.at(i*T+t)) );
        }
    }

    for (int t=0 ; t < T ; t++) {
        SCIPgetTransformedCons( scip, Master->convexity_cstr.at(t), &(Master->convexity_cstr.at(t)) );
    }


    //variables ?
    //pour l'instant on n'a pas besoin de les manipuler a priori

    return SCIP_OKAY;
}


/** Pricing of additional variables if LP is feasible.
 *
 *  - get the values of the dual variables you need
 *  - construct the reduced units costs from dual costs
 *  - find the cheapest production plan for each site
 *  - if this plan has negative reduced cost, add it to the LP
 *
 *  possible return values for *result:
 *  - SCIP_SUCCESS    : at least one improving variable was found, or it is ensured that no such variable exists
 *  - SCIP_DIDNOTRUN  : the pricing process was aborted by the pricer, there is no guarantee that the current LP solution is optimal
 */
//SCIP_DECL_PRICERREDCOST(ObjPricerUCP::scip_redcost)
//{
//    SCIPdebugMsg(scip, "call scip_redcost ...\n");
//    /* set result pointer, see above */
//    *result = SCIP_SUCCESS;
//    /* call pricing routine */
//    pricingUCP(scip,0);
//    return SCIP_OKAY;
//}

SCIP_RETCODE ObjPricerTimeUCP::scip_redcost(SCIP* scip, SCIP_PRICER* pricer, SCIP_Real* lowerbound, SCIP_Bool* stopearly, SCIP_RESULT* result)
{

    SCIPdebugMsg(scip, "call scip_redcost ...\n");

    /* set result pointer, see above */
    *result = SCIP_SUCCESS;

    /* call pricing routine */
    pricingUCP(scip,0);

    return SCIP_OKAY;

}

SCIP_RETCODE ObjPricerTimeUCP::scip_farkas( SCIP* scip, SCIP_PRICER* pricer, SCIP_RESULT* result ){

    SCIPdebugMsg(scip, "call scip_farkas ...\n");

    /* set result pointer, see above */
    *result = SCIP_SUCCESS;

    /* call pricing routine */
    pricingUCP(scip,1);

    return SCIP_OKAY;

}



void ObjPricerTimeUCP::updateDualCosts(SCIP* scip, DualCostsTime & dual_cost, bool Farkas) {
    ///// RECUPERATION DES COUTS DUAUX

    int print = 0 ;
    int n = inst->getn() ;
    int T = inst->getT() ;

    //cout << "solution duale :" << endl ;

    //couts duaux "logical constraint"
    for (int i = 0; i < n; i++) {
        for (int t = 1 ; t < T ; t++) {
            if (!Farkas) {
                dual_cost.Mu.at(i*T+t) = SCIPgetDualsolLinear(scip, Master->logical.at(i*T+t));
            }
            else{
                dual_cost.Mu.at(i*T+t) = SCIPgetDualfarkasLinear(scip, Master->logical.at(i*T+t));
            }
            if (print)
                cout << "mu(" << i <<"," << t <<") = " << dual_cost.Mu[i*T+t] <<endl;
        }
    }

    //couts duaux "min-up constraint"
    for (int i = 0; i < n; i++) {
        int L = inst->getL(i) ;
        for (int t = L ; t < T ; t++) {
            if (!Farkas) {
                dual_cost.Nu.at(i*T+t) = SCIPgetDualsolLinear(scip, Master->min_up.at(i*T+t));
            }
            else{
                dual_cost.Nu.at(i*T+t) = SCIPgetDualfarkasLinear(scip, Master->min_up.at(i*T+t));
            }
            if (print)
                cout << "nu(" << i <<"," << t <<") = " << dual_cost.Nu.at(i*T+t) <<endl;
        }
    }

    //couts duaux "min-down constraint"
    for (int i = 0; i < n; i++) {
        int l = inst->getl(i) ;
        for (int t = l ; t < T ; t++) {
            if (!Farkas) {
                dual_cost.Xi.at(i*T+t) = SCIPgetDualsolLinear(scip, Master->min_down.at(i*T+t));
            }
            else{
                dual_cost.Xi.at(i*T+t) = SCIPgetDualfarkasLinear(scip, Master->min_down.at(i*T+t));
            }
            if (print)
                cout << "xi(" << i <<"," << t <<") = " << dual_cost.Xi.at(i*T+t) <<endl;
        }
    }

    //couts duaux "convexity constraint"
    for (int t = 0 ; t < T ; t++) {
        if (!Farkas) {
            dual_cost.Sigma.at(t) = SCIPgetDualsolLinear(scip, Master->convexity_cstr.at(t));
        }
        else{
            dual_cost.Sigma.at(t) = SCIPgetDualfarkasLinear(scip, Master->convexity_cstr.at(t));
        }
        if (print)
            cout << "sigma(" << t <<") = " << dual_cost.Sigma[t] <<endl;
    }

    if (print) cout << endl ;

}

void ObjPricerTimeUCP::pricingUCP( SCIP*              scip  , bool Farkas             /**< SCIP data structure */)
{
#ifdef OUTPUT_PRICER
    cout<<"**************PRICER************ ";
    // SCIPprintBestSol(scip, NULL, FALSE);
#endif

    int print = 0 ;

    if (print) {

        /// PMR courant et sa solution
        SCIPwriteTransProblem(scip, NULL, NULL, FALSE);

        // cout << "solution du PMR:" << endl ;
        SCIPprintSol(scip, NULL, NULL, FALSE);

        //cout << "solution réalisable:" << endl ;
        SCIPprintBestSol(scip, NULL, FALSE);
    }

    //// Cout duaux
    int T = inst->getT() ;
    int n = inst->getn() ;

    DualCostsTime dual_cost = DualCostsTime(inst) ;
    updateDualCosts(scip, dual_cost, Farkas);


    for (int t = 0 ; t < T ; t++) {

       if (print) cout << "time "<< t << endl;

        ///// MISE A JOUR DES OBJECTIFS DES SOUS PROBLEMES
       // cout << "mise à jour des couts, farkas=" << Farkas << endl;
        (AlgoCplex[t])->updateObjCoefficients(inst, Param, dual_cost, Farkas) ;

        //// CALCUL D'UN PLAN DE COUT REDUIT MINIMUM
        double objvalue = 0 ;
        double realCost=0 ;
        IloNumArray upDownPlan = IloNumArray((AlgoCplex[t])->env, n) ;

        int solutionFound = (AlgoCplex[t])->findUpDownPlan(inst, dual_cost, upDownPlan, objvalue, realCost) ;

        // cout << "solution found: " << solutionFound << endl;
        if (!solutionFound) {
            //PRUNE THE NODE
        }
        if (print) cout << "Minimum reduced cost plan: "<< objvalue << endl ;

        if (print) {

            for (int i=0 ; i < n ; i++) {
                cout << fabs(upDownPlan[i]) << " " ;
            }
            cout << endl ;

        }

        if (SCIPisNegative(scip, objvalue)) {

            MasterTime_Variable* lambda = new MasterTime_Variable(t, upDownPlan, realCost);
            cout << "Plan found for time " << t << " with reduced cost = " << objvalue << " ";
            //// CREATION D'UNE NOUVELLE VARIABLE DANS LE MASTER
            Master->initMasterTimeVariable(scip, inst, lambda) ;

            /* add new variable to the list of variables to price into LP (score: leave 1 here) */
            SCIPaddPricedVar(scip, lambda->ptr, 1.0);

            ///// ADD COEFFICIENTS TO DEMAND, POWER LIMITS and CONVEXITY CONSTRAINTS
            Master->addCoefsToConstraints(scip, lambda, inst) ;
        }
    }

#ifdef OUTPUT_PRICER
    SCIPwriteTransProblem(scip, "ucp.lp", "lp", FALSE);
    cout<<"************END PRICER******************"<<endl;
#endif

}


