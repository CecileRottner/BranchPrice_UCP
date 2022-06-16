\ENCODING=ISO-8859-1
\Problem name: UCP

Minimize
 obj: 25 t_p(0,0) + 381.28855 t_V_0 + 381.28855 t_W_0
Subject To
 _EqTimeSite(0,0)#0: t_V_0 - t_W_0  = 0
 ConvSite(0)#1:      t_V_0 + V_1  = 1
 ConvTime(0)#2:      t_W_0  = 1
 PowerLimit(0,0)#3:  - t_p(0,0) - t_slack_power(0,0) + 42 t_V_0  = 0
 Demand(0)#4:        - t_slack_demand(0) + t_p(0,0) + 30 t_V_0  = 60
Bounds
 0 <= t_slack_demand(0) <= 10000
 0 <= t_p(0,0) <= 42
 0 <= t_slack_power(0,0) <= 42
End
