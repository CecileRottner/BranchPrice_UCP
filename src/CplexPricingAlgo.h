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
  void addBranchingConstraint(); //local to the branch considered

  // Launch Cplex solver and get back an optimal up/down plan
  bool findImprovingSolution(InstanceUCP* inst, const DualCostsTime & Dual, double& objvalue, double & temps_resolution) ; // returns true if an improving solution has been found. objvalue is updated in this case
  void getUpDownPlan(InstanceUCP* inst, const DualCostsTime & Dual, IloNumArray UpDownPlan, double& realCost, bool Farkas) ; //updates UpDownPlan and realCost

};

#endif
