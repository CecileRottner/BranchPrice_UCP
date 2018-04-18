#include "Pricer.h"
#include "scip/cons_linear.h"
#include <map>
#include <vector>
#include <iostream>


//#define OUTPUT_PRICER
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
 *  - construct the reduced-cost arc lengths from these values
 *  - find the shortest admissible tour with respect to these lengths
 *  - if this tour has negative reduced cost, add it to the LP
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

void ObjPricerUCP::pricingUCP(
   SCIP*              scip               /**< SCIP data structure */
   )
{

}


