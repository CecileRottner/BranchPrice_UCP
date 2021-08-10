# Pricing algorithms

Pricing algorithms/solvers functions are located in files :
* CplexPricingAlgo.h (all declarations, should be renamed "PricingAlgo.h")
* CplexPricingAlgo.cpp (frontal Cplex for unit/site subproblems)
* CplexPricingAlgoTime.cpp (frontal Cplex for time subproblems)
* DynProgPricingAlgo.cpp (dynamic programming for unit subproblems -- not implemented for site subproblems)
* DynProgSUSDPricingAlgo.cpp (dynamic programming for unit subproblems where start-up costs depend on the unit's total down time)
* DynProgPricingAlgoTime.cpp (dynamic programming for time subproblems)


## Dual costs classes

Classes representing dual costs of master problem are defined in CplexPricingAlgo.h:
* DualCosts (for unit/site subproblems)
* DualCostsTime (for time subproblems)

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

2 algorithms are implemented in this class:

   (1) DP algorithm for 1 unit with min-up/min-down constraints
   
   (2) DP algorithm for 1 with min-up/min-down constraints and start-up costs depending on the unit's down time

It defines the following methods for DP algorithm (1):
  * **findImprovingSolution**: runs DP algorithm and return Bellman values and predecessor vector
  * **getUpDownPlan**: computes up/down plan from Bellman values and predecessor vector

The same methods, suffixed by SUSD are defined for DP algorithm (2).



### DynProgPricingAlgoTime

Class for solving time subproblems with dynamic programming algorithm.
It is implemented only for the case when production variables are in the master problem : therefore it is a DP algorithm for solving knapsack problems.

Class **DynProgPricingAlgoTime** features attributes :
* **W** : int, capacity of the knapsack to be solved
* **totalBastCost** : double, 
* **init** : vector of size n, indicating for each unit i, if i must be taken in the knapsack (init[i]=1) ou if i must not be taken in the knapsack (init[i] = 0). 
If we are free to choose whether i is taken or not, then init[i]=-1. 
Enables to take branching decisions into account inside dynamic programming.
* **BaseObjCoef** :
* **ObjCoef** :
* **Table** : matrix of size n x W, corresponds to the Bellman table computed by DP algorithm (function findImprovingSolution)

Similar methods are implemented :

  * **findImprovingSolution**: runs DP algorithm and fills Bellman values in **Table**
  * **getUpDownPlan**: computes up/down plan from Bellman values in **Table**
