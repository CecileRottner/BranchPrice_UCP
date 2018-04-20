\ SCIP STATISTICS
\   Problem name     : UCP
\   Variables        : 16 (0 binary, 2 integer, 0 implicit integer, 14 continuous)
\   Constraints      : 10
\   Obj. scale       : 1
\   Obj. offset      : 0
Minimize
 Obj: +1070 all_up_(site_0) +2520 all_up_(site_1)
Subject to
 Demand(0): -1 slack_demand(0) +1 p(0,0) +1 p(1,0) +1 p(2,0) +140 all_up_(site_0) +80 all_up_(site_1)
  +30 all_up_(site_1) = +170
 Demand(1): -1 slack_demand(1) +1 p(0,1) +1 p(1,1) +1 p(2,1) +140 all_up_(site_0) +80 all_up_(site_1)
  +30 all_up_(site_1) = +220
 PowerLimit(0,0): -1 slack_power(0,0) -1 p(0,0) = +0
 PowerLimit(0,1): -1 slack_power(0,1) -1 p(0,1) = +0
 PowerLimit(1,0): -1 slack_power(1,0) -1 p(1,0) = +0
 PowerLimit(1,1): -1 slack_power(1,1) -1 p(1,1) = +0
 PowerLimit(2,0): -1 slack_power(2,0) -1 p(2,0) = +0
 PowerLimit(2,1): -1 slack_power(2,1) -1 p(2,1) = +0
 Convexity(0): +1 all_up_(site_0) = +1
 Convexity(1): +1 all_up_(site_1) = +1
Bounds
 0 <= slack_power(1,0) <= 0
 0 <= slack_power(1,1) <= 0
 0 <= slack_power(2,0) <= 0
 0 <= slack_power(2,1) <= 0
 0 <= slack_demand(0) <= 1000
 0 <= slack_demand(1) <= 1000
 0 <= p(0,0) <= 0
 0 <= p(0,1) <= 0
 0 <= p(1,0) <= 0
 0 <= p(1,1) <= 0
 0 <= p(2,0) <= 0
 0 <= p(2,1) <= 0
 0 <= slack_power(0,0) <= 0
 0 <= slack_power(0,1) <= 0
Generals
 all_up_(site_0) all_up_(site_1)
End
