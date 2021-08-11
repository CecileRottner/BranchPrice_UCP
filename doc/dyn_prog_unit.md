# Dynamic programming algorithm to solve unit subproblems

The set of dynamic programming states consists in couples (t, b), for t in {1, ..., T} and b in {0,1} indicates whether the unit is up or down.

Each transition from (t-1, b) to (t,b) is allowed (except if it contradicts branching decisions), and costs ObjCoefX[i] (the cost for unit i being up at time t).

Transition from (t-L, 0) to (t,1) is allowed, where L is the minimum up time.
It costs L * ObjCoefX[i] because this transition correspond to a start-up of unit i at time t-L+1, and unit i must remain up until time t at least because of minimum up times.

Transition from (t-l, 1) to (t,0) is allowed, where l is the minimum down time.
It costs 0 because this transition correspond to a shut-down of unit i at time t-l+1, and unit i must remain down until time t at least because of minimum down times.





* **Bellman** : matrix of size (T,2). Bellman(t, 0) (resp. Bellman(t, 1)) indicates the minimum cost of an up/down plan for unit s from time 0 to time t, where unit s is down  (resp. up) at time t.

* **Prec** : matrix of size (T,2). Prec(t,0) indicates 

The following methods are defined :

  * **findImprovingSolution**: runs DP algorithm and return Bellman values and predecessor vector
  * **getUpDownPlan**: computes up/down plan from Bellman values and predecessor vector
