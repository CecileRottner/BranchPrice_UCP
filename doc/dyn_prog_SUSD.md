
[Go back to pricing algorithms](pricing_algo.md)

# Dynamic programming algorithm to solve unit subproblems with start-up costs depending on down time

Algorithm implemented in file DynProgPricingAlgoSUSD.cpp.

Methods of class DynProgPricingAlgo defined in CplexPricingAlgo.h.

# Cost function

For each unit i and time step t, the start-up cost of i for a start-up at time t is computed as :

c0(2 - exp(-float(down_duration)/T) )

where down_duration is the total down duration of the unit when it starts up at t.

In commit 3c7b2f8 (branch thibault, not yet in master), the cost function is modified to:
c0(1 - exp(-float(down_duration)/T) )

## Dynamic programming states and transitions

The states and transitions for this DP algo directly correspond to those from the graph of the "Flow formulation" defined p25 of thesis https://hal.archives-ouvertes.fr/tel-02052101/document 

The algorithm is implemented for any unit i.

The set of dynamic programming states consists in couples (t, b), for t in {0, ..., T-1} and b in {0,1} indicates whether unit i starts-up (b=1) or shuts-down (b=0) at time t.
The source is denoted by -1 and the sink is denoted by T.

The set of allowed transitions are described below.
Note that the function checkTransitionSUSD(instance, t1, t2, state) checks that a transtion (t1, not(state)) --> (t2, state) is valid with respect to min-up/down times and branching constraints.



### Transitions from the source

A transition from the source to (t=0,1) is allowed with value 0 : in this case the unit is up at time 0 but will shut down at time 1. The production cost (propportional and fixed) will be taken into account on the next transition.

A transition from the source to (t=0,0) is allowed with value 0 : represents an initial down state which is kept at time 0. 

Transitions from the source to (t,1) are allowed for any t.
Such transition indicates that the unit is up in its initial state, remains up until time t and shuts down at time t+1.

No transition from the source to (t,0) are allowed for t > 0.
Indeed, the total down state must be taken into account when starting-up the first time, therefore if the initial state is 0, then the node (t=0,0) must be used.


### Transitions from t to t'

A transition from (t, 0) to (t',1) means that unit i is up at time t-1, shuts down at time t, remains down from t to t'-1 and starts up and time t'. The cost of such a transition corresponds to the starts-up cost at time t' (after being down from time t to t').


Similarly, a transition from (t, 1) to (t',0) means that unit i is down at time t-1,  starts up at time t, remains up from t to t'-1 and shuts down and time t'. The cost of such a transition correspond to the fixed cost for being up of unit i.

Therefore, a transition (t, 0) to (t', 1) is not allowed if t' - t < l (where l is the minimum down time).
Similarly, a transition (t, 1) to (t', 0) is not allowed if t' - t < L (where L is the minimum up time).

No transition of type (t,0) --> (t',0) or of type (t,1) --> (t',1) are allowed.

### Transitions to the sink
 
Transitions (t, 1) --> sink are allowed : the unit remains up from t to the end of the horizon.
Production costs are taken into account along the corresponding arc.


Transitions (t, 0) --> sink are allowed : the unit remains down from t to the end of the horizon.
Total cost is 0.


## Implementation

* **Bellman** : matrix of size (T,2). Bellman(t, 0) (resp. Bellman(t, 1)) indicates the minimum cost of an up/down plan for unit i from time 0 to time t, where unit i shuts down  (resp. up) at time t.

* **Prec** : matrix of size (T,2). Prec(t,0) indicates which was the last transition used to obtain the min cost Bellman(t, 0) up/down plan from time 0 to t. This vector is useful to construct the subproblem solution given by the Bellman vector.

The following methods are defined :

  * **findImprovingSolution**: runs DP algorithm and fills Bellman values in vector **Bellman** and predecessor vector **Prec**
  * **getUpDownPlan**: fills up/down plan in vector upDownPlan given as argument, using Bellman values and predecessor vector
  * **checkTransitionSUSD**: checks that a transition (prec_time, prec_status ) --> (current_time, curent_status) is valid (with respect to min-up/down constraints and branching decisions)
  * **computeStartUpCosts** : given a transition (prec_time, 0) --> (current_time, 1), computes the associated start-up cost (taking also into account dual values)
