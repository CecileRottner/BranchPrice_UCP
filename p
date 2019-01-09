data/3_5_1_3_3_0_1_1.txt
Sites: [1, 1, 0]
fisrt unit of each site: [0, 1]
First: [1, 1, 1]
FirstG: [0, 1, 2]
SCIP version 5.0.1 [precision: 8 byte] [memory: block] [mode: optimized] [LP solver: CPLEX 12.6.1.0] [GitHash: b1c222a]
Copyright (C) 2002-2018 Konrad-Zuse-Zentrum fuer Informationstechnik Berlin (ZIB)

cost: 1070
cost: 2520
Write init pl
LP Solver <CPLEX 12.6.1.0>: fastmip setting not available -- SCIP parameter has no effect
LP Solver <CPLEX 12.6.1.0>: clock type cannot be set
LP Solver <CPLEX 12.6.1.0>: row representation of the basis not available -- SCIP parameter lp/rowrepswitch has no effect
LP Solver <CPLEX 12.6.1.0>: solution polishing not available -- SCIP parameter lp/solutionpolishing has no effect
LP Solver <CPLEX 12.6.1.0>: refactorization interval not available -- SCIP parameter lp/refactorinterval has no effect
 --------------------- Lock handler ---------------  
transformed problem has 16 variables (0 bin, 2 int, 0 impl, 14 cont) and 10 constraints
     10 constraints of type <linear>

original problem has 28 active (17.5%) nonzeros and 28 (17.5%) check nonzeros

 --------------------- Check handler ---------------  
solution du PMR:
objective value:                            -infinity
 --------------------- Check handler ---------------  
solution du PMR:
objective value:                            -infinity
 --------------------- Check handler ---------------  
solution du PMR:
objective value:                            -infinity
 --------------------- Check handler ---------------  
solution du PMR:
objective value:                            -infinity
presolving:
clique table cleanup detected 0 bound changes

presolved problem has 26 active (16.25%) nonzeros and 26 (16.25%) check nonzeros

presolving (1 rounds: 1 fast, 1 medium, 1 exhaustive):
 0 deleted vars, 0 deleted constraints, 0 added constraints, 0 tightened bounds, 0 added holes, 0 changed sides, 0 changed coefficients
 0 implications, 0 cliques
presolved problem has 16 variables (0 bin, 2 int, 0 impl, 14 cont) and 10 constraints
     10 constraints of type <linear>
Presolving Time: 0.00

 --------------------- Check handler ---------------  
solution du PMR:
objective value:                                 3590
all_up_(site_0)                                     1 	(obj:1070)
all_up_(site_1)                                     1 	(obj:2520)
slack_demand(0)                                    80 	(obj:0)
slack_demand(1)                                    30 	(obj:0)
 time | node  | left  |LP iter|LP it/n| mem |mdpt |frac |vars |cons |cols |rows |cuts |confs|strbr|  dualbound   | primalbound  |  gap   
* 0.0s|     1 |     0 |     0 |     - | 471k|   0 |   0 |  16 |  10 |  16 |  10 |   0 |   0 |   0 |      --      | 3.590000e+03 |    Inf 
**************PRICER************
solution du PMR:
objective value:                                 3590
all_up_(site_0)                                     1 	(obj:1070)
all_up_(site_1)                                     1 	(obj:2520)
slack_demand(0)                                    80 	(obj:0)
slack_demand(1)                                    30 	(obj:0)
solution réalisable:
objective value:                                 3590
all_up_(site_0)                                     1 	(obj:1070)
all_up_(site_1)                                     1 	(obj:2520)
slack_demand(0)                                    80 	(obj:0)
slack_demand(1)                                    30 	(obj:0)
solution duale :
nu: 0
nu: 0
nu: 0
nu: 0
nu: 0
nu: 0
mu: -0
mu: -0
sigma: 1070
sigma: 2520

site 0
for site 0; 
obj value without sigma: 0
Minimum reduced cost plan: -1070
0 
0 

site 1
for site 1; 
obj value without sigma: 0
Minimum reduced cost plan: -2520
0 0 
0 0 

************END PRICER******************
  0.0s|     1 |     0 |     1 |     - | 471k|   0 |   2 |  18 |  10 |  18 |  10 |   0 |   0 |   0 |      --      | 3.590000e+03 |    Inf 
**************PRICER************
solution du PMR:
objective value:                     2902.72727272727
all_up_(site_0)                                     1 	(obj:1070)
all_up_(site_1)                     0.727272727272727 	(obj:2520)
slack_demand(0)                                    50 	(obj:0)
V_3                                 0.272727272727273 	(obj:0)
solution réalisable:
objective value:                                 3590
all_up_(site_0)                                     1 	(obj:1070)
all_up_(site_1)                                     1 	(obj:2520)
slack_demand(0)                                    80 	(obj:0)
slack_demand(1)                                    30 	(obj:0)
solution duale :
nu: 0
nu: 0
nu: 0
nu: 0
nu: 0
nu: 0
mu: -0
mu: 22.9091
sigma: -2137.27
sigma: 0

site 0
for site 0; 
obj value without sigma: -2572.27
Minimum reduced cost plan: -435
0 
1 

site 1
for site 1; 
obj value without sigma: -947.727
Minimum reduced cost plan: -947.727
0 0 
1 0 

************END PRICER******************
  0.0s|     1 |     0 |     2 |     - | 471k|   0 |   3 |  20 |  10 |  20 |  10 |   0 |   0 |   0 |      --      | 3.590000e+03 |    Inf 
**************PRICER************
solution du PMR:
objective value:                     2310.39772727273
all_up_(site_0)                                     1 	(obj:1070)
all_up_(site_1)                     0.272727272727273 	(obj:2520)
V_3                                 0.102272727272727 	(obj:0)
V_5                                             0.625 	(obj:885)
solution réalisable:
objective value:                                 3590
all_up_(site_0)                                     1 	(obj:1070)
all_up_(site_1)                                     1 	(obj:2520)
slack_demand(0)                                    80 	(obj:0)
slack_demand(1)                                    30 	(obj:0)
solution duale :
nu: 0
nu: 0
nu: 0
nu: 0
nu: 0
nu: 0
mu: 11.8466
mu: 11.0625
sigma: -2137.27
sigma: 0

site 0
for site 0; 
obj value without sigma: -2137.27
Minimum reduced cost plan: 0
1 
1 

site 1
for site 1; 
obj value without sigma: -182.727
Minimum reduced cost plan: -182.727
1 0 
1 0 

************END PRICER******************
  0.0s|     1 |     0 |     3 |     - | 471k|   0 |   2 |  21 |  10 |  21 |  10 |   0 |   0 |   0 |      --      | 3.590000e+03 |    Inf 
**************PRICER************
solution du PMR:
objective value:                             2241.875
all_up_(site_0)                                     1 	(obj:1070)
V_5                                             0.625 	(obj:885)
V_6                                             0.375 	(obj:1650)
solution réalisable:
objective value:                                 3590
all_up_(site_0)                                     1 	(obj:1070)
all_up_(site_1)                                     1 	(obj:2520)
slack_demand(0)                                    80 	(obj:0)
slack_demand(1)                                    30 	(obj:0)
solution duale :
nu: 0
nu: 0
nu: 0
nu: 0
nu: 0
nu: 0
mu: 9.5625
mu: 11.0625
sigma: -1817.5
sigma: 0

site 0
for site 0; 
obj value without sigma: -1817.5
Minimum reduced cost plan: 0
1 
1 

site 1
for site 1; 
obj value without sigma: 0
Minimum reduced cost plan: 0
1 0 
1 0 

************END PRICER******************
  0.0s|     1 |     0 |     3 |     - | 471k|   0 |   2 |  21 |  10 |  21 |  10 |   0 |   0 |   0 | 2.241875e+03 | 3.590000e+03 |  60.13%
 --------------------- Check handler ---------------  
solution du PMR:
objective value:                             2241.875
all_up_(site_0)                                     1 	(obj:1070)
V_5                                             0.625 	(obj:885)
V_6                                             0.375 	(obj:1650)
solution fractionnaire
 --------------------- Sepalp handler ---------------  
 --------------------- Check handler ---------------  
solution du PMR:
objective value:                                 2720
all_up_(site_0)                                     1 	(obj:1070)
slack_demand(0)                                    50 	(obj:0)
V_6                                                 1 	(obj:1650)
i 0.0s|     1 |     0 |     7 |     - | 479k|   0 |   0 |  21 |  10 |  21 |  10 |   0 |   0 |   0 | 2.241875e+03 | 2.720000e+03 |  21.33%
**************PRICER************
solution du PMR:
objective value:                             2241.875
all_up_(site_0)                                     1 	(obj:1070)
V_5                                             0.625 	(obj:885)
V_6                                             0.375 	(obj:1650)
solution réalisable:
objective value:                                 2720
all_up_(site_0)                                     1 	(obj:1070)
slack_demand(0)                                    50 	(obj:0)
V_6                                                 1 	(obj:1650)
solution duale :
nu: 0
nu: 0
nu: 0
nu: 0
nu: 0
nu: 0
mu: 9.5625
mu: 11.0625
sigma: -1817.5
sigma: 0

site 0
for site 0; 
obj value without sigma: -1817.5
Minimum reduced cost plan: 0
1 
1 

site 1
for site 1; 
obj value without sigma: 0
Minimum reduced cost plan: 0
1 0 
1 0 

************END PRICER******************
  0.0s|     1 |     0 |     7 |     - | 479k|   0 |   2 |  21 |  10 |  21 |  10 |   0 |   0 |   0 | 2.241875e+03 | 2.720000e+03 |  21.33%
 --------------------- Sepalp handler ---------------  
 --------------------- Branching Rule EXECLP ---------------  
Nombre de noeuds actuel : 1
Consdata null
Branch on var x(1, 0)  of value : 0.375
 ------ CREATE A CONSTRAINT ASSOCIATED TO A NODE   ---------------  
 ------ END CREATION  ---------------  
 ------ CREATE A CONSTRAINT ASSOCIATED TO A NODE   ---------------  
 ------ END CREATION  ---------------  
  0.0s|     1 |     2 |     7 |     - | 479k|   0 |   2 |  21 |  10 |  21 |  10 |   0 |   0 |   0 | 2.241875e+03 | 2.720000e+03 |  21.33%
 --------------------- Active handler ---------------  
 --------------------- Fin Active handler ---------------  
**************PRICER************
solution du PMR:
objective value:                             2241.875
all_up_(site_0)                                     1 	(obj:1070)
V_5                                             0.625 	(obj:885)
V_6                                             0.375 	(obj:1650)
solution réalisable:
objective value:                                 2720
all_up_(site_0)                                     1 	(obj:1070)
slack_demand(0)                                    50 	(obj:0)
V_6                                                 1 	(obj:1650)
solution duale :
nu: 0
nu: 0
nu: 0
nu: 0
nu: 0
nu: 0
mu: 9.5625
mu: 11.0625
sigma: -1817.5
sigma: 0

site 0
for site 0; 
obj value without sigma: -1817.5
Minimum reduced cost plan: 0
1 
1 

site 1
for site 1; 
obj value without sigma: 0
Minimum reduced cost plan: 0
0 0 
0 0 

************END PRICER******************
 --------------------- Sepalp handler ---------------  
 --------------------- Branching Rule EXECLP ---------------  
Nombre de noeuds actuel : 2
Consdata non null
Branch on var x(1, 0)  of value : 0.375
 ------ CREATE A CONSTRAINT ASSOCIATED TO A NODE   ---------------  
 ------ END CREATION  ---------------  
 ------ CREATE A CONSTRAINT ASSOCIATED TO A NODE   ---------------  
 ------ END CREATION  ---------------  
 --------------------- Active handler ---------------  
 --------------------- Fin Active handler ---------------  
**************PRICER************
solution du PMR:
objective value:                             2241.875
all_up_(site_0)                                     1 	(obj:1070)
V_5                                             0.625 	(obj:885)
V_6                                             0.375 	(obj:1650)
solution réalisable:
objective value:                                 2720
all_up_(site_0)                                     1 	(obj:1070)
slack_demand(0)                                    50 	(obj:0)
V_6                                                 1 	(obj:1650)
solution duale :
nu: 0
nu: 0
nu: 0
nu: 0
nu: 0
nu: 0
mu: 9.5625
mu: 11.0625
sigma: -1817.5
sigma: 0

site 0
for site 0; 
obj value without sigma: -1817.5
Minimum reduced cost plan: 0
1 
1 

site 1
for site 1; 
obj value without sigma: 0
Minimum reduced cost plan: 0
0 0 
0 0 

************END PRICER******************
 --------------------- Sepalp handler ---------------  
 --------------------- Branching Rule EXECLP ---------------  
Nombre de noeuds actuel : 3
Consdata non null
Branch on var x(1, 0)  of value : 0.375
 ------ CREATE A CONSTRAINT ASSOCIATED TO A NODE   ---------------  
 ------ END CREATION  ---------------  
 ------ CREATE A CONSTRAINT ASSOCIATED TO A NODE   ---------------  
 ------ END CREATION  ---------------  

SCIP Status        : solving was interrupted [node limit reached]
Solving Time (sec) : 0.03
Solving Nodes      : 3
Primal Bound       : +2.72000000000000e+03 (2 solutions)
Dual Bound         : +2.24187500000000e+03
Gap                : 21.33 %
 --------------------- Check handler ---------------  
solution du PMR:
objective value:                             2241.875
all_up_(site_0)                                     1 	(obj:1070)
V_5                                             0.625 	(obj:885)
V_6                                             0.375 	(obj:1650)
solution fractionnaire
best solution is not feasible in original problem
Found incumbent of value 3155.000000 after 0.00 sec. (0.01 ticks)
Tried aggregator 2 times.
MIP Presolve eliminated 28 rows and 17 columns.
MIP Presolve modified 2 coefficients.
Aggregator did 2 substitutions.
All rows and columns eliminated.
Presolve time = 0.00 sec. (0.03 ticks)

Root node processing (before b&c):
  Real time             =    0.00 sec. (0.03 ticks)
Parallel b&c, 8 threads:
  Real time             =    0.00 sec. (0.00 ticks)
  Sync time (average)   =    0.00 sec.
  Wait time (average)   =    0.00 sec.
                          ------------
Total (root+branch&cut) =    0.00 sec. (0.03 ticks)
1 0 1 
1 1 0 

VALEUR OPTIMALE A TROUVER: 2390
