\ SCIP STATISTICS
\   Problem name     : t_UCP
\   Variables        : 9 (0 binary, 7 integer, 0 implicit integer, 2 continuous)
\   Constraints      : 10
\   Obj. scale       : 1
\   Obj. offset      : 0
Minimize
 Obj: +1070 t_all_up_(site_0) +2520 t_all_up_(site_1) +635 V_4 +885 V_5 +1650 V_6
Subject to
 Demand(0): +140 t_all_up_(site_0) +110 t_all_up_(site_1) -1 t_slack_demand(0) +80 V_6 = +170
 Demand(1): +140 t_all_up_(site_0) +110 t_all_up_(site_1) -1 t_slack_demand(1) +140 V_4 +80 V_5 +80 V_6
  = +220
 PowerLimit(0,0): = +0
 PowerLimit(0,1): = +0
 PowerLimit(1,0): = +0
 PowerLimit(1,1): = +0
 PowerLimit(2,0): = +0
 PowerLimit(2,1): = +0
 Convexity(0): +1 t_all_up_(site_0) +1 V_2 +1 V_4 = +1
 Convexity(1): +1 t_all_up_(site_1) +1 V_3 +1 V_5 +1 V_6 = +1
Bounds
 0 <= t_slack_demand(1) <= 1000
 0 <= t_slack_demand(0) <= 1000
Generals
 t_all_up_(site_0) t_all_up_(site_1) V_2 V_3 V_4 V_5 V_6
End
