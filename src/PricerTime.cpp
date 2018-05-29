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

    AlgoCplex = vector<CplexPricingAlgo*>(inst->getS(), NULL) ;

    for (int s=0 ; s < inst->getS() ; s++) {
        AlgoCplex[s] = new CplexPricingAlgo(inst, s) ;
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
//    int T = inst->getT() ;
//    // demand constraints
//    for (int t = 0 ; t < T ; t++) {
//        SCIPgetTransformedCons(scip, Master->demand_cstr[t], &(Master->demand_cstr[t]));
//    }

//    //power limits
//    for (int t = 0 ; t < T ; t++) {
//        for (int i = 0 ; i < inst->getn() ; i++) {
//            SCIPgetTransformedCons(scip, Master->power_limits[i*T+t], &(Master->power_limits[i*T+t]));
//        }
//    }

//    //convexity constraints
//    for (int s = 0 ; s < inst->getS() ; s++) {
//        SCIPgetTransformedCons(scip, Master->convexity_cstr[s], &(Master->convexity_cstr[s]));
//    }


//    if (Param.Ramp) {
//        for (int t = 1; t < T; t++) {
//            for (int i = 0 ; i < inst->getn() ; i++) {
//                SCIPgetTransformedCons(scip, Master->ramp_up[i*T+t], &(Master->ramp_up[i*T+t]));
//                SCIPgetTransformedCons(scip, Master->ramp_down[i*T+t], &(Master->ramp_down[i*T+t]));
//            }
//        }
//    }


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



void ObjPricerTimeUCP::updateDualCosts(SCIP* scip, DualCosts & dual_cost, bool Farkas) {
    ///// RECUPERATION DES COUTS DUAUX

//    int print = 0 ;
//    int n = inst->getn() ;
//    int T = inst->getT() ;
//    int S = inst->getS() ;

//    //cout << "solution duale :" << endl ;
//    //couts duaux des power limits
//    for (int i = 0; i < n; i++) {
//        for (int t = 0 ; t < T ; t++) {
//            if (!Farkas) {
//                dual_cost.Nu[i*T+t] = SCIPgetDualsolLinear(scip, Master->power_limits[i*T+t]);
//            }
//            else{
//                dual_cost.Nu[i*T+t] = SCIPgetDualfarkasLinear(scip, Master->power_limits[i*T+t]);
//            }
//            if (print)
//                cout << "nu(" << i <<"," << t <<") = " << dual_cost.Nu[i*T+t] <<endl;
//        }
//    }

//    //couts duaux des ramp
//    //RAMPSTUFF
//    if (Param.Ramp) {
//        for (int i = 0; i < n; i++) {
//            for (int t = 1 ; t < T ; t++) {
//                if (!Farkas) {
//                    dual_cost.Phi[i*T+t] = SCIPgetDualsolLinear(scip, Master->ramp_up[i*T+t]);
//                    dual_cost.Psi[i*T+t] = SCIPgetDualsolLinear(scip, Master->ramp_down[i*T+t]);
//                }
//                else{
//                    dual_cost.Phi[i*T+t] = SCIPgetDualfarkasLinear(scip, Master->ramp_up[i*T+t]);
//                    dual_cost.Psi[i*T+t] = SCIPgetDualfarkasLinear(scip, Master->ramp_down[i*T+t]);
//                }

//               if (print) cout << "phi(" << i <<"," << t <<") = " << dual_cost.Phi[i*T+t] <<endl;
//               if (print) cout << "psi(" << i <<"," << t <<") = " << dual_cost.Psi[i*T+t] <<endl;
//            }
//        }
//    }
//    //couts duaux demande
//    for (int t = 0 ; t < T ; t++) {
//        if (!Farkas) {
//            dual_cost.Mu[t] = SCIPgetDualsolLinear(scip, Master->demand_cstr[t]);
//        }
//        else{
//            dual_cost.Mu[t] = SCIPgetDualfarkasLinear(scip, Master->demand_cstr[t]);
//        }
//        if (print) cout << "mu: " << dual_cost.Mu[t] <<endl;
//    }

//    //couts duaux contrainte convexité
//    for (int s = 0 ; s < S ; s++) {
//        if (!Farkas) {
//            dual_cost.Sigma[s] = SCIPgetDualsolLinear(scip, Master->convexity_cstr[s]);
//        }
//        else{
//            dual_cost.Sigma[s] = SCIPgetDualfarkasLinear(scip, Master->convexity_cstr[s]);
//        }
//        if (print) cout << "sigma: " << dual_cost.Sigma[s] <<endl;
//    }

//    if (print) cout << endl ;

}

void ObjPricerTimeUCP::pricingUCP( SCIP*              scip  , bool Farkas             /**< SCIP data structure */)
{
#ifdef OUTPUT_PRICER
    cout<<"**************PRICER************ ";
    // SCIPprintBestSol(scip, NULL, FALSE);
#endif

    int print = 0 ;

//    /// PMR courant et sa solution
//    SCIPwriteTransProblem(scip, NULL, NULL, FALSE);

//   // cout << "solution du PMR:" << endl ;
//    SCIPprintSol(scip, NULL, NULL, FALSE);

//    //cout << "solution réalisable:" << endl ;
//    SCIPprintBestSol(scip, NULL, FALSE);

    //// Cout duaux
//    int T = inst->getT() ;
//    int S = inst->getS() ;

//    DualCosts dual_cost = DualCosts(inst) ;
//    updateDualCosts(scip, dual_cost, Farkas);


//    for (int s = 0 ; s < S ; s++) {

//       //cout << "site "<< s << endl;

//        ///// MISE A JOUR DES OBJECTIFS DES SOUS PROBLEMES
//       // cout << "mise à jour des couts, farkas=" << Farkas << endl;
//        (AlgoCplex[s])->updateObjCoefficients(inst, Param, dual_cost, Farkas) ;

//        //// CALCUL D'UN PLAN DE COUT REDUIT MINIMUM
//        double objvalue = 0 ;
//        IloNumArray upDownPlan = IloNumArray((AlgoCplex[s])->env, inst->nbUnits(s)*T) ;
//        int solutionFound = (AlgoCplex[s])->findUpDownPlan(inst, dual_cost, upDownPlan, objvalue) ;

//        // cout << "solution found: " << solutionFound << endl;
//        if (!solutionFound) {
//            //PRUNE THE NODE
//        }
//        if (print) cout << "Minimum reduced cost plan: "<< objvalue << endl ;

//        if (print) {
//            for (int t=0 ; t < T ; t++)  {
//                for (int i=0 ; i < inst->nbUnits(s) ; i++) {
//                    cout << fabs(upDownPlan[i*T+t]) << " " ;
//                }
//                cout << endl ;
//            }
//            cout << endl ;
//        }

//        if (SCIPisNegative(scip, objvalue)) {

//            Master_Variable* lambda = new Master_Variable(s, upDownPlan);
//            cout << "Plan found for site " << s << " with reduced cost = " << objvalue << " ";
//            //// CREATION D'UNE NOUVELLE VARIABLE DANS LE MASTER
//            Master->initMasterVariable(scip, inst, lambda) ;

//            /* add new variable to the list of variables to price into LP (score: leave 1 here) */
//            SCIPaddPricedVar(scip, lambda->ptr, 1.0);

//            ///// ADD COEFFICIENTS TO DEMAND, POWER LIMITS and CONVEXITY CONSTRAINTS
//            Master->addCoefsToConstraints(scip, lambda, inst) ;
//        }
//    }

#ifdef OUTPUT_PRICER
    SCIPwriteTransProblem(scip, "ucp.lp", "lp", FALSE);
    cout<<"************END PRICER******************"<<endl;
#endif

}


