# Pricing algorithms

Pricing algorithms/solvers classes declarations and definitions are located in files :
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

* CplexPricingAlgo : class for solving unit/site subproblems with frontal Cplex
Defines the following methods:
  * **updateObjCoeficients**: takes new dual costs as input and updates the subproblem objective function
  * **findUpDownPlan**: launches Cplex to find a minimum reduced cost column
