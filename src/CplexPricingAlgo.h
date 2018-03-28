#ifndef PRICER
#define PRICER

#include <ilcplex/ilocplex.h>

#include <vector>
#include <list>

#include "InstanceUCP.h"
#include "Master.h"

using namespace std;
using namespace scip;

class DualCosts {
public:
    vector<double> Mu ;
    vector<double> Nu ;
    vector<double> Sigma ;

    DualCosts(const InstanceUCP & inst) ;
};

class CplexPricingAlgo {
 public:

  int Site ;
  IloEnv   env;
  IloModel model;
  IloObjective obj;
  IloCplex cplex;

  IloBoolVarArray x;
  IloBoolVarArray u;

  vector<double> BaseObjCoefX ;

  void initialize(const InstanceUCP & inst, int site);

  void updateObjCoefficients(const InstanceUCP & inst, const DualCosts & Dual);
  void addBranchingConstraint(); //local to the branch considered

  // Launch Cplex solver and get back an optimal up/down plan
  bool findUpDownPlan(const InstanceUCP & inst, IloNumArray UpDownPlan, double & objvalue);

};



#endif
