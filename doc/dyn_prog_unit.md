[Go back to pricing algorithms](pricing_algo.md)

# Dynamic programming algorithm to solve unit subproblems

Algorithm implemented in file DynProgPricingAlgo.cpp.

Methods of class DynProgPricingAlgo defined in CplexPricingAlgo.h.

## Dynamic programming states and transitions
The algorithm is implemtend for any unit i.
The set of dynamic programming states consists in couples (t, b), for t in {1, ..., T} and b in {0,1} indicates whether unit i is up or down.

Each transition from (t-1, b) to (t, b) is allowed (except if it contradicts branching decisions), and costs ObjCoefX[i] (the cost for unit i being up at time t).

Transition from (t-L, 0) to (t,1) is allowed (except if it contradicts branching decisions), where L is the minimum up time.
It costs L * DualCosts.ObjCoefX[i]  + DualCosts.ObjCoefU(i) because this transition correspond to a start-up of unit i at time t-L+1, and unit i must remain up until time t at least because of minimum up times.

Transition from (t-l, 1) to (t,0) is allowed (except if it contradicts branching decisions), where l is the minimum down time.
It costs 0 because this transition correspond to a shut-down of unit i at time t-l+1, and unit i must remain down until time t at least because of minimum down times.



## Implementation

* **Bellman** : matrix of size (T,2). Bellman(t, 0) (resp. Bellman(t, 1)) indicates the minimum cost of an up/down plan for unit s from time 0 to time t, where unit s is down  (resp. up) at time t.

* **Prec** : matrix of size (T,2). Prec(t,0) indicates which was the last transition used to obtain the min cost Bellman(t, 0) up/down plan from time 0 to t. This vector is useful to construct the subproblem solution given by the Bellman vector.

The following methods are defined :

  * **findImprovingSolution**: runs DP algorithm and fills Bellman values in vector **Bellman** and predecessor vector **Prec**
  * **getUpDownPlan**: fills up/down plan in vector upDownPlan given as argument, using Bellman values and predecessor vector
