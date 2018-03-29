#ifndef PRICER
#define PRICER

#include "objscip/objscip.h"
#include "scip/pub_var.h"

#include <vector>
#include <list>

#include "InstanceUCP.h"
#include "Master.h"
#include "CplexPricingAlgo.h"

using namespace std;
using namespace scip;

/** pricer class */
// hérité d'une classe de SCIP
class ObjPricerUCP : public ObjPricer{
public:


    InstanceUCP* inst ;
    Master_Model* Master ;
    vector<CplexPricingAlgo*> AlgoCplex;


   /** Constructs the pricer object with the data needed */
   ObjPricerUCP(
      SCIP*                               scip,        /**< SCIP pointer */
      const char*                         p_name,       /**< name of pricer */
      Master_Model*                       M,
      InstanceUCP*                        inst
      );

   /** Destructs the pricer object. */
   virtual ~ObjPricerUCP();

   /** initialization method of variable pricer (called after problem was transformed) */
   virtual SCIP_DECL_PRICERINIT(scip_init);

   /** reduced cost pricing method of variable pricer for feasible LPs */
   virtual SCIP_DECL_PRICERREDCOST(scip_redcost);

   /** performs pricing */
   void pricingUCP(
      SCIP*              scip               /**< SCIP data structure */
      );



};

#endif
