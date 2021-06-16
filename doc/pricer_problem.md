# Pricer problem

Pricer classes declarations and definitions are located in files :
* Pricer.h
* Pricer.cpp (for unit/site decompositions)
* PricerTime.cpp (for time decompositions)
* PricerDouble.cpp (for unit/site+time decompositions)

## Pricer problem classes

### ObjPricerUCP (virtual)

Class __ObjPricerUCP__ (virtual) contains indicators common to all pricer problems, regardless of decomposition type / pricing solver :

* number of columns of type "unit/site"
* number of columns of type "time"
* total dual cost

Two virtual functions are defined:
* addVarBound(c) : adds constraint c to the subproblem (where c is a bound constraint on a given variable)
* removeVarBound(c) : removes constraint c from the subproblem

### Classes inherited from ObjPricerUCP

 All classes inherited from ObjPricerUCP are structured similarly. Each corresponds to a given decomposition structure (unit/site, time, double)
 
 They both contain the following attributes, depending on the decomposition type : 
 * pointer to the master model
 * for each algorithm available to solve the pricing problem (eg: frontal Cplex, dynamic programming, ...) : a vector of pointers to this algorithm class, instanciated for each subproblem
 (for exemple, for each unit in a unit decomposition)
 
 and SCIP methods redefined specifically for each pricer class:
* SCIP_DECL_PRICERINIT: initialization method of the pricer, called after problem was transformed. Gets new references to master constraints after presolve. 
* scip_redcost: call to pricing algorithms (method pricingUCP) when master problem is feasible
* scip_farkas: when master is infeasible, call pricing algorithms with modified costs, in order to generate columns making the master feasible
 
 
 They also all define the following methods:
 * **updateDualCost** : after master is solved, updates reduced costs (or Farkas costs if argument farkas=true) from SCIP and put them in the vector given as argument
 * **pricingUCP** performs pricing:
    * Calls updateDualCosts to obtain new duals costs, using DualCosts object
    * Updates objective coefficients in subproblem, using dual costs
    * Calls pricing algorithm chosen (frontal Cplex, dynamic programming) to find a minimum reduced cost column
    * If the reduced cost of the optimal column is negative, then :
       * a new master variable is created (using constructor of corresponding master variable class)
       * the new variable is added to the master problem (function SCIPaddPricedVar)
       * the new variable is added to master constraints with coeficients depending on the column (with master method addCoefsToConstraints)
 

#### ObjPricerSite

The pricer class used in a unit/site decomposition

#### ObjPricerTimeUCP

The pricer class used in a time decomposition

#### ObjPricerDouble

The pricer class used in a double (unit/site + time) decomposition




