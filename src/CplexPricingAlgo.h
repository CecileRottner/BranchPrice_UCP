#ifndef CPLEXPRICER
#define CPLEXPRICER

#include <ilcplex/ilocplex.h>

#include <vector>
#include <list>

#include "InstanceUCP.h"
#include "Master.h"
#include "Process.h"

using namespace std;
using namespace scip;

class DualCosts {
public:
    vector<double> Mu ;
    vector<double> Nu ;
    vector<double> Sigma ;
    vector<double> Phi ;
    vector<double> Psi ;

    DualCosts(InstanceUCP* inst) ;
};


class CplexPricingAlgo {
 public:

  const Parameters Param ;
  int Site ;
  IloEnv   env;
  IloModel model;
  IloObjective obj;
  IloCplex cplex;

  IloBoolVarArray x;
  IloBoolVarArray u;

  vector<double> BaseObjCoefX ;

  CplexPricingAlgo(InstanceUCP* inst, const Parameters & Param, int site);

  void updateObjCoefficients(InstanceUCP* inst, const Parameters & Param, const DualCosts & Dual, bool Farkas);
  void addBranchingConstraint(); //local to the branch considered

  // Launch Cplex solver and get back an optimal up/down plan
  bool findUpDownPlan(InstanceUCP* inst, const DualCosts & Dual, IloNumArray UpDownPlan, double & objvalue);

};


////////////////////////////////////////////////////
////////// DECOMPOSITION PAR PAS DE TEMPS //////////
////////////////////////////////////////////////////


class DualCostsTime {
public:
    vector<double> Mu ;
    vector<double> Nu ;
    vector<double> Xi ;
    vector<double> Sigma ;

    DualCostsTime(InstanceUCP* inst) ;
};

class CplexPricingAlgoTime {
 public:

  Parameters Param ;
  int time ;
  IloEnv   env;
  IloModel model;
  IloObjective obj;
  IloCplex cplex;

  IloBoolVarArray x;
  IloNumVarArray p ;

  vector<double> BaseObjCoefX ;

  CplexPricingAlgoTime(InstanceUCP* inst, const Parameters & par, int t);

  void updateObjCoefficients(InstanceUCP* inst, const Parameters & Param, const DualCostsTime & Dual, bool Farkas);

  // Launch Cplex solver and get back an optimal up/down plan
  bool findImprovingSolution(InstanceUCP* inst, const DualCostsTime & Dual, double& objvalue, double & temps_resolution, int exact) ; // returns true if an improving solution has been found. objvalue is updated in this case
  void getUpDownPlan(InstanceUCP* inst, const DualCostsTime & Dual, IloNumArray UpDownPlan, double& realCost, double & totalProd, bool Farkas) ; //updates UpDownPlan and realCost
};


class DynProgPricingAlgoTime { // codé dans le cas Pmin=Pmax pour voir si c'est intéressant. Cas où D et les puissances sont entiers
 public:

  Parameters Param ;
  MasterTime_Model* Master;
  int time ;
  IloEnv   env;

  int W ;
  double totalBaseCost ;

  vector<int> init ; // init[i]=0 si i pas pris dans le sac à dos, init[i]=1 si i pris dans le sac à dos, si ça reste à déterminer: -1

  vector<double> BaseObjCoefX ;
  vector<double> ObjCoefX ;
  vector<double> Table ;

  DynProgPricingAlgoTime(InstanceUCP* inst, MasterTime_Model* Master, const Parameters & par, int t); // initialise les vecteurs, et W

  void updateObjCoefficients(InstanceUCP* inst, const Parameters & Param, const DualCostsTime & Dual, bool Farkas);

  // Launch Cplex solver and get back an optimal up/down plan
  bool findImprovingSolution(InstanceUCP* inst, const DualCostsTime & Dual, double& objvalue, double & temps_resolution, int exact) ; // returns true if an improving solution has been found. objvalue is updated in this case
  void getUpDownPlan(InstanceUCP* inst, const DualCostsTime & Dual, IloNumArray UpDownPlan, double& realCost, double & totalProd, bool Farkas) ; //updates UpDownPlan and realCost

};


#endif
