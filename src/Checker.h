#ifndef CPLEXCHECK
#define CPLEXCHECK

#include <ilcplex/ilocplex.h>

#include <vector>
#include <list>

#include "Process.h"
#include "InstanceUCP.h"

using namespace std;


class CplexChecker {
 public:

  InstanceUCP* inst ;
  const Parameters Param ;
  IloEnv   env;
  IloModel model;
  IloCplex cplex;

  IloExpr cost;

  IloBoolVarArray x;
  IloBoolVarArray u;
  IloNumVarArray pp ;
  IloBoolVarArray d;
  IloBoolVarArray u_temps;

  double PrimalBound;
  double DualBound ;  
  double PrimalBoundLowBound;
  double DualBoundLowBound ;
  double nbNodes;
  double nbNodesLowBound;
  double cpuTime ;
  double cpuTimeLowBound ;
  double gap ;


  double LRValue ;
  double LRCplexVal ;
  double valHeuristicCplex ;

  CplexChecker(InstanceUCP* inst, const Parameters & param) ;
  double getIntegerObjValue() ;
  double useLowBound(double lowbound) ;
  double getLRValue() ;
  double getLRCplex() ;
  void CplexPrimalHeuristic(IloNumArray solution, IloNumArray solution_p) ;

  double printSolution();
  void checkSolution(const vector<double> & x_frac);
  void checkSolution(const vector<double> & x_frac, const vector<double> & p_frac);

};



#endif
