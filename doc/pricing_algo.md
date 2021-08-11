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
It is implemented only for the case when production variables are in the master problem : therefore it is a DP algorithm for solving knapsack problems. The algorithm implemtend is this one : https://en.wikipedia.org/wiki/Knapsack_problem#0-1_knapsack_problem

The subproblem to solve for a given time step $t$ has the following form :

$\min \sum_{i=1}^n c^i x^i$ 
s. t. $\sum_{i=1}^n P_{max}^i x^i \geq D_t$ 

Note that a solution to this problem provides an up/down plan $x$ for which there exists a production plan satisfying the demand $D_t$.

We transform to a knapsack problem as follows using variable
 $\overline{x}^i = 1 - x^i$ :

$\max \sum_{i=1}^n c^i \overline{x}^i$ 
s. t. $\sum_{i=1}^n P_{max}^i \overline{x}^i \leq \sum_{i=1}^n P_{max}^i - D_t$ 

Class **DynProgPricingAlgoTime** features attributes :

* **W** : int, capacity of the knapsack to be solved.
Defined here for time step t as $W = \sum_{i=1}^n P_{max}^i - D_t$

* **BaseObjCoef** : for each unit i, BaseObjCoef[i] corresponds to the production cost of unit i at Pmax : $c_f^i + c_p^i P_{max}^i$ (computed in constructor)
* **ObjCoef** : for each unit i, ObjCoef[i] = $c^i$ corresponds to the reduced cost of unit i : BaseObjCoef[i] + costs coming from dual values (computed by function updateObjCoefficients)
* **totalBastCost** : double, sum of BaseObjCoef[i] for all i (computed in constructor)

* **init** : vector of size n, indicating for each unit i, if i must be taken in the knapsack (init[i]=1) ou if i must not be taken in the knapsack (init[i] = 0). 
If we are free to choose whether i is taken or not, then init[i]=-1. 
This enables to take branching decisions into account inside dynamic programming.

* **Table** : matrix of size n x W, corresponds to the Bellman table computed by DP algorithm (function findImprovingSolution)

Similar methods are implemented :

  * **updateObjCoefficients**: fills ObjCoef using BaseObjCoef and dual values from Master problem
  * **findImprovingSolution**: runs DP algorithm and fills Bellman values in Table
  * **getUpDownPlan**: computes up/down plan from Bellman values in Table
