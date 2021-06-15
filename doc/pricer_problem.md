# Pricer problem

## ObjPricerUCP (virtual)

Class __ObjPricerUCP__ (virtual) contains indicators common to all pricer problems, regardless of decomposition type / pricer solver :

* number of columns of type "unit/site"
* number of columns of type "time"
* total dual cost

Two virtual functions are defined:
* addVarBound(c) : adds constraint c to the subproblem (where c is a bound constraint on a given variable)
* removeVarBound(c) : removes constraint c from the subproblem

## Classes inherited from ObjPricerUCP

 Classes inherited from ObjPricerUCP work similarly. Each corresponds to a given decomposition structure (unit/site, time, double)
 
 They both contain the following attributes, depending on the decomposition type : 
 * pointer to the master model
 * for each algorithm available to solve the pricing problem (eg: frontal Cplex, dynamic programming, ...) : a vector of pointers to this algorithm class, instanciated for each subproblem
 (for exemple, for each unit in a unit decomposition)
 
 and SCIP methods redefined specifically for each pricer class:
* SCIP_DECL_PRICERINIT: initialization method of variable pricer (called after problem was transformed)
* scip_redcost: appel aux algos de pricing, lorsque le problème maître est faisable
* scip_farkas: quand le problème maître est infaisable, appel aux algos de pricing avec coûts modifiés afin de générer des colonnes qui rendent le master faisable
 
 

### ObjPricerSite

The pricer class used in a unit/site decomposition

### ObjPricerTimeUCP

The pricer class used in a time decomposition

### ObjPricerDouble

The pricer class used in a double (unit/site + time) decomposition

