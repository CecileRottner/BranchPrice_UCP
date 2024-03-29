# Dynamic programming algorithm to solve time subproblems

## Class DynProgPricingAlgoTime



Class for solving time subproblems with dynamic programming algorithm.
It is implemented only for the case when production variables are in the master problem : therefore it is a DP algorithm for solving knapsack problems. The algorithm implemtend is this one : https://en.wikipedia.org/wiki/Knapsack_problem#0-1_knapsack_problem

The subproblem to solve for a given time step t has the following form :

<img src="https://render.githubusercontent.com/render/math?math=\min \sum_{i=1}^n c^i x^i">

<img src="https://render.githubusercontent.com/render/math?math=\sum_{i=1}^n P_{max}^i x^i \geq D_t">

Note that a solution to this problem provides an up/down plan x for which there exists a production plan satisfying the demand Dt.

We transform to a knapsack problem as follows using variable <img src="https://render.githubusercontent.com/render/math?math=\overline{x}^i = 1 - x^i">:

<img src="https://render.githubusercontent.com/render/math?math=\max \sum_{i=1}^n c^i \overline{x}^i">

<img src="https://render.githubusercontent.com/render/math?math=\sum_{i=1}^n P_{max}^i \overline{x}^i \leq \sum_{i=1}^n P_{max}^i - D_t">

Class **DynProgPricingAlgoTime** features attributes :

* **W** : int, capacity of the knapsack to be solved.
Defined here for time step t as <img src="https://render.githubusercontent.com/render/math?math=W = \sum_{i=1}^n P_{max}^i - D_t">

* **BaseObjCoef** : for each unit i, BaseObjCoef[i] corresponds to the production cost of unit i at Pmax : <img src="https://render.githubusercontent.com/render/math?math=c^i_f %2B c_p^i P_{max}^i"> (computed in constructor)
* **ObjCoef** : for each unit i, ObjCoef[i] = <img src="https://render.githubusercontent.com/render/math?math=c^i"> corresponds to the reduced cost of unit i : BaseObjCoef[i] + costs coming from dual values (computed by function updateObjCoefficients)
* **totalBastCost** : double, sum of BaseObjCoef[i] for all i (computed in constructor)

* **init** : vector of size n, indicating for each unit i, if i must be taken in the knapsack (init[i]=1) ou if i must not be taken in the knapsack (init[i] = 0). 
If we are free to choose whether i is taken or not, then init[i]=-1. 
This enables to take branching decisions into account inside dynamic programming.

* **Table** : matrix of size (n+1) x (W+1), corresponds to the Bellman table computed by DP algorithm (function findImprovingSolution). 
As in the [referenced algorithm](https://en.wikipedia.org/wiki/Knapsack_problem#0-1_knapsack_problem), Table(i,w) is the maximum value that can be attained with weight less than or equal to w using the first i items.
Note that in the code, Table(i,w) is stored in Table[i*(W+1)+w].

Methods implemented :

  * **updateObjCoefficients**: fills ObjCoef using BaseObjCoef and dual values from Master problem
  * **findImprovingSolution**: runs DP algorithm and fills Bellman values in Table
  * **getUpDownPlan**: puts the knapsack optimal solution in vector upDownPlan given as argument, using Bellman values in Table
