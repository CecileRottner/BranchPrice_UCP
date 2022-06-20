# Pricing algorithms

Pricing algorithms/solvers functions are located in files :
* CplexPricingAlgo.h (header for all pricing algorithms, should be renamed "PricingAlgo.h")
* CplexPricingAlgo.cpp (frontal Cplex for unit/site subproblems)
* CplexPricingAlgoTime.cpp (frontal Cplex for time subproblems)
* DynProgPricingAlgo.cpp (dynamic programming for unit subproblems -- not implemented for site subproblems)
* DynProgSUSDPricingAlgo.cpp (dynamic programming for unit subproblems where start-up costs depend on the unit's total down time)
* DynProgPricingAlgoTime.cpp (dynamic programming for time subproblems)


## Dual costs classes

Classes representing dual costs of master problem are defined in CplexPricingAlgo.h:

* DualCosts (for unit/site subproblems)

* DualCostsTime (for time subproblems)


Class **DualCosts** for **unit/site** subproblems features in particular the following attributes :

  * **BaseObjCoef** : for each unit i, BaseObjCoef[i* T + t] corresponds to cost for unit i being up at time t : <img src="https://render.githubusercontent.com/render/math?math=c^i_f %2B c_p^i P_{min}^i"> (computed in constructor)
  * **ObjCoefX** : for each unit i, ObjCoefX[i* T + t] corresponds to the reduced cost of variable x(i,t) : BaseObjCoef[i] + costs coming from dual values (computed by method computeObjCoefficients)
  * **ObjCoefX** : for each unit i, ObjCoefU[i* T + t] corresponds to the reduced cost of variable u(i,t) (computed by method computeObjCoefficients)

Corresponding attributes for time subproblems are features in class DynProgPricingAlgoTime.

Each class contains a vector of dual values associated to each master constraint.

Note that double (unit/site + time) decompositions uses both classes DualCosts and DualCostsTime : each class contains a vector Omega containing dual values of the (in)equality constraint linking time columns to unit/site columns. Other constraints in the master problem are either relative to a unit/site decomposition or to a time decomposition, thus the corresponding dual cost class is used to keep track of the dual costs.



## Pricing algorithm classes

### CplexPricingAlgo

Class for solving unit/site subproblems with frontal Cplex. 

It defines the following methods:
  * **updateObjCoeficients**: takes new dual costs as input and updates the subproblem objective function
  * **findUpDownPlan**: launches Cplex to find a minimum reduced cost column. If one is found, it is given in argument "UpDownPlan"
  * **AddSSBI** (optional): adds symmetry-breaking to the subproblem (site subproblem with identical units) before it is solved by Cplex 


### DynProgPricingAlgo

Class for solving unit subproblems with dynamic programming algorithm (DP for site subproblems is not implemented yet). 

The following attributes are defined :

* **s** : int, refers to the site concerned by the subproblem. Note that the DP algorithm is implemented only for sites composed of 1 unit, i.e. site s will correspond here to unit s

* **branchingDecision**s : vector of size n. For time step t, branchingDecisions[t] = 0 (resp. 1) if unit s has been fixed to 0 (resp. 1) by branching at time t. If unit s is not fixed by branching at time t, then branchingDecisions[t] = 8.


2 algorithms are implemented in this class:

   (1) DP algorithm for 1 unit with min-up/min-down constraints : see details [here](dyn_prog_unit.md)
   
   Note that the algorithm only works for min up time L < T (no loss of generality)
   
   (2) DP algorithm for 1 with min-up/min-down constraints and start-up costs depending on the unit's down time





### DynProgPricingAlgoTime

Class for solving time subproblems with dynamic programming algorithm.
See details [here](dyn_prog_time.md)
