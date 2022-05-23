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

class DualCostsTime {
public:
    vector<double> Mu ;
    vector<double> Nu ;
    vector<double> Xi ;
    vector<double> Sigma ;

    //SSBI
    vector<double> Epsilon ;
    vector<double> Delta ;

    vector<double> Omega ; // contraintes d'égalités time/site

    DualCostsTime(InstanceUCP* inst) ;
};


class DualCosts {
public:
    vector<double> Mu ;
    vector<double> Nu ; // power limits
    vector<double> Sigma ;
    vector<double> Eta ; // intra site dual cost
    vector<double> Phi ; // ramp dual cost
    vector<double> Psi ;

    vector<double> Zeta ;
    vector<double> Ksi ;
    vector<double> Theta ;


    vector<double> Omega ; // contraintes d'égalités time/site


    //Coefficients de x et u
    vector<double> BaseObjCoefX ; //ne change pas au cours des itérations: calculé dans le constructeur
    vector<double> ObjCoefX ;

    vector<double> ObjCoefU ;
    vector<double> ObjCoefP ;

    DualCosts(InstanceUCP* inst, const Parameters & Param) ;

    //Computes objective coefficients of x and u once dual values are updated
    // Depends on dualCostsTime in the case of double decomposition where min-up / min-down constraints are used for stabilization
    void computeObjCoef(InstanceUCP* inst, const Parameters & Param, bool Farkas, const DualCostsTime & dualTime) ;
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
  IloNumVarArray p;


  double cpuTime ;

  CplexPricingAlgo(InstanceUCP* inst, const Parameters & Param, int site);

  void updateObjCoefficients(InstanceUCP* inst, const Parameters & Param, const DualCosts & Dual, bool Farkas);

  // Launch Cplex solver and get back an optimal up/down plan
  bool findUpDownPlan(InstanceUCP* inst, const DualCosts & Dual, IloNumArray UpDownPlan, double & objvalue);

  void AddSSBI(IloEnv env, IloModel model, IloBoolVarArray x, IloBoolVarArray u, int site, InstanceUCP* inst) ;

};



///// Programmation dynamique pour une unité seulement //////
///////// Pas adapté à la décomposition start-up ////////////
// en construction

class DynProgPricingAlgo {
 public:

  Parameters Param ;
  Master_Model* Master;
  int Site ;
  IloEnv   env;

  vector<double> branchingDecisions ;

  vector<double> Bellman ;
  vector<double> Prec ;

  DynProgPricingAlgo(InstanceUCP* inst, Master_Model* Master, const Parameters & par, int Site); // initialise les vecteurs

  bool findImprovingSolution(InstanceUCP* inst, const DualCosts & Dual, double& objvalue) ;
  //computes Bellman and Prec (predecessor) vectors
  // returns true if an improving solution has been found. objvalue is updated in this case

  void getUpDownPlan(InstanceUCP* inst, IloNumArray UpDownPlan) ;
  //updates UpDownPlan and realCost

  //checks that the dynamic programming transition given as argument can be used w.r.t. to the branching decisions
  //returns true if this is the case
  bool checkTransition(int prec_time, int current_time, int prec_status, int current_status) ;

////// Start-up-shut-down dynamic programming algorithm ////

  int time_prec_sink;
  int status_prec_sink;
  bool checkTransitionSUSD(InstanceUCP* inst, int prec_time, int current_time, int current_status) ;
  double computeStartUpCosts(InstanceUCP* inst,  const DualCosts & Dual, int prec_time, int current_time);
// checks that the transition exists in the graph (in particular wrt min up and down times) and that they are feasible wrt the branching decisions
  bool findImprovingSolutionSUSD(InstanceUCP* inst, const DualCosts & Dual, double& objvalue) ;
  void getUpDownPlanSUSD(InstanceUCP* inst,  IloNumArray UpDownPlan) ;

};

////////////////////////////////////////////////////
////////// DECOMPOSITION PAR PAS DE TEMPS //////////
////////////////////////////////////////////////////



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
  bool findImprovingSolution(InstanceUCP* inst, const DualCostsTime & Dual, double& objvalue, double & temps_resolution, int exact) ;
  // returns true if an improving solution has been found. objvalue is updated in this case
  void getUpDownPlan(InstanceUCP* inst, const DualCostsTime & Dual, IloNumArray UpDownPlan, IloNumArray PowerPlan, double& realCost, double & totalProd, bool Farkas) ; //updates UpDownPlan and realCost
};

class DynProgPricingAlgoTime { // codé dans le cas Pmin=Pmax pour voir si c'est intéressant. Cas où D et les puissances sont entiers
 public:

  Parameters Param ;
  Master_Model* Master;
  int time ;
  IloEnv   env;

  int W ; // sum of Pmax - demand, used by DynProgPricingAlgoTimeNoPower

  vector<int> init ; // init[i]==0 si i pas pris dans le sac à dos, init[i]=1 si i pris dans le sac à dos, si ça reste à déterminer: -1

  vector<double> BaseObjCoefX ;
  vector<double> ObjCoefX ;

  DynProgPricingAlgoTime(InstanceUCP* inst, Master_Model* M, const Parameters & par, int t) : Param (par){
    Master=M ;
    time=t;
  }; 

  virtual void updateObjCoefficients(InstanceUCP* inst, const Parameters & Param, const DualCostsTime & Dual, bool Farkas) = 0;

  virtual bool findImprovingSolution(InstanceUCP* inst, const DualCostsTime & Dual, double& objvalue, double & temps_resolution, int exact) = 0 ;
  // computes Bellman table (vector Table)
  // returns true if an improving solution has been found. objvalue is updated in this case

  virtual void getUpDownPlan(InstanceUCP* inst, const DualCostsTime & Dual, IloNumArray UpDownPlan, IloNumArray PowerPlan, double& realCost, double & totalProd, bool Farkas) = 0;
  //updates UpDownPlan and realCost

};


class DynProgPricingAlgoTimeNoPower : public DynProgPricingAlgoTime { // codé dans le cas Pmin=Pmax pour voir si c'est intéressant. Cas où D et les puissances sont entiers
 public:

  double totalBaseCost ;

  vector<double> Table ;

  DynProgPricingAlgoTimeNoPower(InstanceUCP* inst, Master_Model* Master, const Parameters & par, int t); // initialise les vecteurs, et W

  void updateObjCoefficients(InstanceUCP* inst, const Parameters & Param, const DualCostsTime & Dual, bool Farkas);

  bool findImprovingSolution(InstanceUCP* inst, const DualCostsTime & Dual, double& objvalue, double & temps_resolution, int exact) ;
  // computes Bellman table (vector Table)
  // returns true if an improving solution has been found. objvalue is updated in this case

  void getUpDownPlan(InstanceUCP* inst, const DualCostsTime & Dual, IloNumArray UpDownPlan, IloNumArray PowerPlan, double& realCost, double & totalProd, bool Farkas) ;
  //updates UpDownPlan and realCost

};

class DynProgPricingAlgoTimePower : public DynProgPricingAlgoTime { // dans le cas où l'on considère les knapsacks continus en temps
 public:

  vector<double> power; // power[i] représente la production associée à l'unité

  int Dt ; //demande au temps time

  vector<double> BaseObjCoefP ;
  vector<double> ObjCoefP ;

  vector<vector<double>> Table ;

  int pivotUnit ; // unité dont la production ne vaut pas Pmin ou Pmax ; vaut -1 si ca reste à déterminer

  DynProgPricingAlgoTimePower(InstanceUCP* inst, Master_Model* Master, const Parameters & par, int t); // initialise les vecteurs, et W

  void updateObjCoefficients(InstanceUCP* inst, const Parameters & Param, const DualCostsTime & Dual, bool Farkas);

  bool findImprovingSolution(InstanceUCP* inst, const DualCostsTime & Dual, double& objvalue, double & temps_resolution, int exact) ;
  // returns true if an improving solution has been found. objvalue is updated in this case

  void getUpDownPlan(InstanceUCP* inst, const DualCostsTime & Dual, IloNumArray UpDownPlan, IloNumArray PowerPlan, double& realCost, double & totalProd, bool Farkas) ;
  //updates UpDownPlan and realCost

};

#endif
