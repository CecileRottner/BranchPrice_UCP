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
ObjPricerUCP::ObjPricerUCP(
        SCIP*                                scip,          /**< SCIP pointer */
        const char*                         pp_name,      /**< name of pricer */
        Master_Model*                        M,
        InstanceUCP*                        instance
        ):
    ObjPricer(scip, pp_name, "Find production plans with negative reduced costs for each site.", 0, TRUE)
{


    inst=instance ;
    Master=M;

    AlgoCplex = vector<CplexPricingAlgo*>(inst->getS(), NULL) ;

    for (int s=0 ; s < inst->getS() ; s++) {
        AlgoCplex[s] = new CplexPricingAlgo(inst, s) ;
    }

}


/** Destructs the pricer object. */
ObjPricerUCP::~ObjPricerUCP()
{
    cout<<"Destructeur du pricer"<<endl;
}

/** initialization method of variable pricer (called after problem was transformed)
 *
 *  Because SCIP transformes the original problem in preprocessing, we need to get the references to
 *  the variables and constraints in the transformed problem from the references in the original
 *  problem.
 */
SCIP_DECL_PRICERINIT(ObjPricerUCP::scip_init)
{

    int T = inst->getT() ;
    // demand constraints
    for (int t = 0; t < T; t++) {
        SCIPgetTransformedCons(scip, Master->demand_cstr[t], &(Master->demand_cstr[t]));
    }

    //power limits
    for (int t = 0; t < T; t++) {
        for (int i = 0 ; i < inst->getn() ; i++) {
            SCIPgetTransformedCons(scip, Master->power_limits[i*T+t], &(Master->power_limits[i*T+t]));
        }
    }

    //convexity constraints
    for (int s = 0 ; s < inst->getS() ; s++) {
        SCIPgetTransformedCons(scip, Master->convexity_cstr[s], &(Master->convexity_cstr[s]));
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
SCIP_DECL_PRICERREDCOST(ObjPricerUCP::scip_redcost)
{
    SCIPdebugMsg(scip, "call scip_redcost ...\n");
    /* set result pointer, see above */
    *result = SCIP_SUCCESS;
    /* call pricing routine */
    pricingUCP(scip);
    return SCIP_OKAY;
}

void ObjPricerUCP::pricingUCP( SCIP*              scip               /**< SCIP data structure */)
{
#ifdef OUTPUT_PRICER
    cout<<"**************PRICER************"<<endl;
   // SCIPprintBestSol(scip, NULL, FALSE);
#endif

    /// PMR courant et sa solution
    SCIPwriteTransProblem(scip, NULL, NULL, FALSE);
    cout << "solution du PMR:" << endl ;

    SCIPprintSol(scip, NULL, NULL, FALSE);


    //// Cout duaux
    int n = inst->getn() ;
    int T = inst->getT() ;
    int S = inst->getS() ;

    DualCosts dual_cost = DualCosts(inst) ;

    ///// RECUPERATION DES COUTS DUAUX
    ///
    cout << "solution duale :" << endl ;
    //couts duaux des power limits
    for (int i = 0; i < n; i++) {
        for (int t = 0 ; t < T ; t++) {
            dual_cost.Nu[i*T+t] = SCIPgetDualsolLinear(scip, Master->power_limits[i*T+t]);
           cout << "nu: " << dual_cost.Nu[i*T+t] <<endl;
        }
    }

    //couts duaux demande
    for (int t = 0 ; t < T ; t++) {
        dual_cost.Mu[t] = SCIPgetDualsolLinear(scip, Master->demand_cstr[t]);
         cout << "mu: " << dual_cost.Mu[t] <<endl;
    }

    //couts duaux contrainte convexité
    for (int s = 0 ; s < S ; s++) {
        dual_cost.Sigma[s] = SCIPgetDualsolLinear(scip, Master->convexity_cstr[s]);
        cout << "sigma: " << dual_cost.Sigma[s] <<endl;
    }


    cout << endl ;

    for (int s = 0 ; s < S ; s++) {

        cout << "site "<< s << endl;

        ///// MISE A JOUR DES OBJECTIFS DES SOUS PROBLEMES

        (AlgoCplex[s])->updateObjCoefficients(inst, dual_cost) ;


        //// CALCUL D'UN PLAN DE COUT REDUIT MINIMUM
        double objvalue = 0 ;
        IloNumArray upDownPlan = IloNumArray((AlgoCplex[s])->env, inst->nbUnits(s)*T) ;
        (AlgoCplex[s])->findUpDownPlan(inst, dual_cost, upDownPlan, objvalue) ;


        cout << "Minimum reduced cost plan: "<< objvalue << endl ;

        for (int t=0 ; t < T ; t++)  {
            for (int i=0 ; i < inst->nbUnits(s) ; i++) {
                cout << fabs(upDownPlan[i*T+t]) << " " ;
            }
            cout << endl ;
        }

        cout << endl ;

        if (SCIPisNegative(scip, objvalue)) {

            //cout << "Plan found for site " << s << " with reduced cost = " << objvalue <<" : ";

            //// CREATION D'UNE NOUVELLE VARIABLE DANS LE MASTER
            char var_name[255];
            SCIPsnprintf(var_name, 255, "V_%d",Master->L_var.size());
            SCIPdebugMsg(scip, "new variable <%s>\n", var_name);

            /* create the new variable: Use upper bound of infinity such that we do not have to care about
             * the reduced costs of the variable in the pricing. The upper bound of 1 is implicitly satisfied
             * due to the set partitioning constraints.
             */


            Master_Variable *var = new Master_Variable(s, upDownPlan);
            var->computeCost(inst);
            double cost= var->cost;
            SCIPcreateVar(scip, &(var->ptr), var_name,
                          0.0,                     // lower bound
                          SCIPinfinity(scip),      // upper bound
                          cost,                     // objective
                          SCIP_VARTYPE_INTEGER,    // variable type
                          false, false, NULL, NULL, NULL, NULL, NULL);

            /* add new variable to the list of variables to price into LP (score: leave 1 here) */
            SCIPaddPricedVar(scip, var->ptr, 1.0);

            ///// ADD COEFFICIENTS TO DEMAND, POWER LIMITS and CONVEXITY CONSTRAINTS
            Master->addCoefsToConstraints(scip, var, inst) ;

            //// Add new variable to the list
            Master->L_var.push_back(var);
        }
    }


#ifdef OUTPUT_PRICER
  SCIPwriteTransProblem(scip, "ucp.lp", "lp", FALSE);
  cout<<"************END PRICER******************"<<endl;
#endif



}


