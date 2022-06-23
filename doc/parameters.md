# Choosing resolution parameters

Choosing the solving parameters is done by the **met** argument, which must be an integer with at least 3 digits.

Each digit defines a different aspect of the solving.


### First digit

* **1** : Unit decomposition
* **2** : Time-step decomposition
* **3** : Double decomposition for $P_{min}=P_{max}$
* **4** : Double decomposition for $P_{min} \neq P_{max}$ with production variables $p$ in the time-step subproblem
* **5** : Double decomposition for $P_{min} \neq P_{max}$ with production variables $p$ in the master problem


### Second digit

* **0** : Solve at root node (relaxation)
* **1** : Branch & Price


### Third digit

* **0** : Initialization with columns corresponding to every unit on all the time
* **1** : Heuristic initialization (columns given by CPLEX)


### Fourth digit : double decomposition-specific parameters

See [code file](/src/Process.cpp) for details.