
[Go back to pricing algorithms](pricing_algo.md)

# Dynamic programming algorithm to solve unit subproblems with start-up costs depending on down time

Algorithm implemented in file DynProgPricingAlgoSUSD.cpp.

Methods of class DynProgPricingAlgo defined in CplexPricingAlgo.h.

## Dynamic programming states and transitions
The algorithm is implemtend for any unit i.
The set of dynamic programming states consists in couples (t, b), for t in {1, ..., T} and b in {0,1} indicates whether unit i is start-up (b=1) or shuts-down (b=0).

A transition (t, 0) to (t',1) means that unit i is up at time t-1, shuts down at time t, remains down from t to t'-1 and starts-up and time t'.




## Implementation

* **Bellman** : matrix of size (T,2). Bellman(t, 0) (resp. Bellman(t, 1)) indicates the minimum cost of an up/down plan for unit s from time 0 to time t, where unit s is down  (resp. up) at time t.

* **Prec** : matrix of size (T,2). Prec(t,0) indicates which was the last transition used to obtain the min cost Bellman(t, 0) up/down plan from time 0 to t. This vector is useful to construct the subproblem solution given by the Bellman vector.

The following methods are defined :

  * **findImprovingSolution**: runs DP algorithm and fills Bellman values in vector **Bellman** and predecessor vector **Prec**
  * **getUpDownPlan**: fills up/down plan in vector upDownPlan given as argument, using Bellman values and predecessor vector
