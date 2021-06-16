### General data

* **n**: number of units
* **T**: number of time periods
* **D**: power demand at each time step

* **K**: number of unit type (= number of different units, useful for symmetrical instances)
* **ns**: number of sites


### Units data

* **Init**: initial status of each unit (up = 1 or down = 0)
* **L**: minimum up time of each unit
* **l**: minimum down time of each unit
* **Pmin**: minimum power output for each up unit
* **Pmax**: maximum power output for each up unit
* **cf**: fixed cost of each unit
* **c0**: start-up cost of each unit
* **cp**: proportional cost of each unit
* **nk**: type of each unit (ie: if 2 units have the same type, then they are identical)
* **S**: site index of each unit (ie: nk[i] is the index of the site of unit i)

