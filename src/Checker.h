#ifndef CPLEXCHECK
#define CPLEXCHECK

#include <ilcplex/ilocplex.h>

#include <vector>
#include <list>

#include "InstanceUCP.h"

using namespace std;


class CplexChecker {
 public:

  InstanceUCP* inst ;
  IloEnv   env;
  IloModel model;
  IloCplex cplex;

  IloBoolVarArray x;
  IloBoolVarArray u;
  IloNumVarArray pp ;

  double noIntraObj;
  double ObjValue ;
  double LRCplexVal ;

  CplexChecker(InstanceUCP* inst) ;
  double checkValue();
  void checkSolution(const vector<double> & x_frac);

};



#endif
