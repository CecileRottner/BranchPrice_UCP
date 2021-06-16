# Pricing algorithms

Pricing algorithms/solvers classes declarations and definitions are located in files :
* CplexPricingAlgo.h (all declarations, should be renamed "PricingAlgo.h")
* CplexPricingAlgo.cpp (frontal Cplex for unit/site subproblems)
* CplexPricingAlgoTime.cpp (frontal Cplex for time subproblems)
* DynProgPricingAlgo.cpp (dynamic programming for unit subproblems -- not implemented for site subproblems)
* DynProgSUSDPricingAlgo.cpp (dynamic programming for unit subproblems where start-up costs depend on the unit's total down time)
* DynProgPricingAlgoTime.cpp (dynamic programming for time subproblems)
