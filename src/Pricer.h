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
class ObjPricerUCP : public ObjPricer {
public:
    InstanceUCP* inst ;
    const Parameters Param ;

    int timeColumns;
    int unitColumns;

    double totalDualCost;

    ObjPricerUCP(
            SCIP*                               scip,
            const char*                         pp_name,
            InstanceUCP*                        i,
            const Parameters &                  p
            ) :
        ObjPricer(scip, pp_name, "Find production plans with negative reduced costs for each subproblem", 0, TRUE),
        inst(i),
        Param(p)
    {}

    virtual void addVarBound(SCIP_ConsData* consdata) = 0;
    virtual void removeVarBound(SCIP_ConsData* consdata) = 0;

//    /** initialization method of variable pricer (called after problem was transformed) */
//    virtual SCIP_DECL_PRICERINIT(scip_init) {};

//    /** reduced cost pricing method of variable pricer for feasible LPs */
//   // virtual SCIP_DECL_PRICERREDCOST(scip_redcost);
//    virtual  SCIP_RETCODE scip_redcost(SCIP* scip, SCIP_PRICER* pricer, SCIP_Real* lowerbound, SCIP_Bool* stopearly, SCIP_RESULT* result) override {}


//    // recherche d'une variable pour la faisabilité du PMR et insertion si trouvée
//    virtual  SCIP_RETCODE scip_farkas(SCIP* scip, SCIP_PRICER* pricer, SCIP_RESULT* result) override {}
    virtual ~ObjPricerUCP() {}
};


class ObjPricerSite : public ObjPricerUCP {
public:


    MasterSite_Model* Master ;
    vector<CplexPricingAlgo*> AlgoCplex;
    vector<DynProgPricingAlgo*> AlgoDynProg;

    int iteration;
   /** Constructs the pricer object with the data needed */
   ObjPricerSite(
      SCIP*                               scip,        /**< SCIP pointer */
      const char*                         p_name,       /**< name of pricer */
      MasterSite_Model*                       M,
      InstanceUCP*                        inst,
      const Parameters &                  param
      );

   /** Destructs the pricer object. */
   virtual ~ObjPricerSite();

   /** initialization method of variable pricer (called after problem was transformed) */
   virtual SCIP_DECL_PRICERINIT(scip_init);

   /** reduced cost pricing method of variable pricer for feasible LPs */
  // virtual SCIP_DECL_PRICERREDCOST(scip_redcost);
   virtual  SCIP_RETCODE scip_redcost(SCIP* scip, SCIP_PRICER* pricer, SCIP_Real* lowerbound, SCIP_Bool* stopearly, SCIP_RESULT* result) override;


   // recherche d'une variable pour la faisabilité du PMR et insertion si trouvée
   virtual  SCIP_RETCODE scip_farkas(SCIP* scip, SCIP_PRICER* pricer, SCIP_RESULT* result) override;

   /*put dual costs or farkas cost in vector dual_cost*/
   void updateDualCosts(SCIP* scip, DualCosts & dual_cost, bool Farkas) ;

   void addVarBound(SCIP_ConsData* consdata) ;
   void removeVarBound(SCIP_ConsData* consdata) ;

   /** performs pricing */
   void pricingUCP(
      SCIP*              scip,               /**< SCIP data structure */
      bool               Farkas
      );

};


////////////////////////////////////////////////////
////////// DECOMPOSITION PAR PAS DE TEMPS //////////
////////////////////////////////////////////////////

class ObjPricerTimeUCP : public ObjPricerUCP {
public:

    MasterTime_Model* Master ;
    vector<CplexPricingAlgoTime*> AlgoCplex;
    vector<DynProgPricingAlgoTime*> AlgoDynProg;

    vector<int> TimeSolNotFound ;
    int lastTimeStep;
    int nbCallsToCplex ;

   /** Constructs the pricer object with the data needed */
   ObjPricerTimeUCP(
      SCIP*                               scip,        /**< SCIP pointer */
      const char*                         p_name,       /**< name of pricer */
      MasterTime_Model*                       M,
      InstanceUCP*                        inst,
      const Parameters &                  param
      );

   /** Destructs the pricer object. */
   virtual ~ObjPricerTimeUCP();

   /** initialization method of variable pricer (called after problem was transformed) */
   virtual SCIP_DECL_PRICERINIT(scip_init);

   /** reduced cost pricing method of variable pricer for feasible LPs */
  // virtual SCIP_DECL_PRICERREDCOST(scip_redcost);
   virtual  SCIP_RETCODE scip_redcost(SCIP* scip, SCIP_PRICER* pricer, SCIP_Real* lowerbound, SCIP_Bool* stopearly, SCIP_RESULT* result) override;


   // recherche d'une variable pour la faisabilité du PMR et insertion si trouvée
   virtual  SCIP_RETCODE scip_farkas(SCIP* scip, SCIP_PRICER* pricer, SCIP_RESULT* result) override;

   /*put dual costs or farkas cost in vector dual_cost*/
   void updateDualCosts(SCIP* scip, DualCostsTime & dual_cost, bool Farkas) ;

   void addVarBound(SCIP_ConsData* consdata) ;
   void removeVarBound(SCIP_ConsData* consdata) ;

   /** performs pricing */
   void pricingUCP(
      SCIP*              scip,               /**< SCIP data structure */
      bool               Farkas
      );

};



//////////////////////////////////////////
////////// DOUBLE DECOMPOSITION //////////
//////////////////////////////////////////

class ObjPricerDouble : public ObjPricerUCP {
public:


    MasterDouble_Model* Master ;
    vector<CplexPricingAlgo*> AlgoCplex_site;
    vector<DynProgPricingAlgo*> AlgoDynProg_site;


    vector<CplexPricingAlgoTime*> AlgoCplex_time;
    vector<DynProgPricingAlgoTime*> AlgoDynProg_time;



    int iteration;
   /** Constructs the pricer object with the data needed */
   ObjPricerDouble(
      SCIP*                               scip,        /**< SCIP pointer */
      const char*                         p_name,       /**< name of pricer */
      MasterDouble_Model*                       M,
      InstanceUCP*                        inst,
      const Parameters &                  param
      );

   /** Destructs the pricer object. */
   virtual ~ObjPricerDouble();

   /** initialization method of variable pricer (called after problem was transformed) */
   virtual SCIP_DECL_PRICERINIT(scip_init);

   /** reduced cost pricing method of variable pricer for feasible LPs */
  // virtual SCIP_DECL_PRICERREDCOST(scip_redcost);
   virtual  SCIP_RETCODE scip_redcost(SCIP* scip, SCIP_PRICER* pricer, SCIP_Real* lowerbound, SCIP_Bool* stopearly, SCIP_RESULT* result) override;


   // recherche d'une variable pour la faisabilité du PMR et insertion si trouvée
   virtual  SCIP_RETCODE scip_farkas(SCIP* scip, SCIP_PRICER* pricer, SCIP_RESULT* result) override;

   /*put dual costs or farkas cost in vector dual_cost*/
   void updateDualCosts_time(SCIP* scip, DualCostsTime & dual_cost, bool Farkas) ;
   void updateDualCosts_site(SCIP* scip, DualCosts & dual_cost, bool Farkas) ;

   void addVarBound(SCIP_ConsData* consdata) ;
   void removeVarBound(SCIP_ConsData* consdata) ;

   /** performs pricing */
   void pricingUCP(
      SCIP*              scip,               /**< SCIP data structure */
      bool               Farkas
      );

};


#endif
