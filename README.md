# Branch&Price for the Unit Commitment Problem

Runs with SCIP 5.0.1 and Cplex


## What does this code do?

Several decomposition structures for the UCP are available in this framework :

* Unit/site decompositions :
  * classical unit decomposition : coupling constraints (demand and intra-site) and production constraints are dualized and the unit's technical constraints remain in the subproblems (1 per unit)
  * site decomposition : demand and production constraints are dualized , other constraints remain in the subproblems (1 per site)
  * start-up decomposition : only start-up and shut-down decisions are taken in the subproblems, other decisions are made in the master problem
  * Residual demand decomposition : residual coupling demand constraints are added to subproblems corresponding to each site
  * Power decomposition : only demand constraints are dualized, production constraints are put in the subproblem. Useful when the subproblems feature ramp-up/down constraints 

* Time decomposition : 

* Unit+Time decomposition :

See Chapter 8 of https://hal.archives-ouvertes.fr/tel-02052101/document for more details on these decomposition structures.

Each of these decomposition structures can be used together with cut generation for the UCP (interval-up set inequalities) as well as some symmetry-breaking techniques.

It can be run as:
- a column generation algorithm
- a Branch&Price algorithm
- a Price&Branch algorithm

See code doc on :
- [Master problem](doc/master_model.md)
- [Pricer problem](doc/pricer_problem.md)
- [Pricing algorithms](doc/pricing_algo.md)

## Compile and execute
- Update Cplex and SCIP directory paths in file cplex_scip_dir.mk
- Launch script compil.sh
- Launch script exec.sh

## Instances

In directory "data/small_UCP_data", some UCP instances were generated and named 
```bash
n_T_p_demand_sym_bin_intra_id.txt
```

where :
- n : number of production units
- T : number of time periods
- p is a parameter for instance generation (has no importance here)
- demand : type of production demand (3 means the classical form "two peak per day")
- sym : level of symmetry in the instance (0 means no symmetry, and x means there are in average n/x identical units)
- bin : equals 1 if instances are such that Pmin=Pmax for all production units (production levels are then binary), equals 0 otherwise
- intra : equals 1 if instances are subject to "intra site" constraints (ie units are grouped on geographical sites and there is at most one start up per time period on each site)
- id : unique identifier of the instance

Details on instance format are [there](doc/instance_format.md)

## Arguments

Command line 
```bash
./bin/SCIP_UCP_BP.linux.x86_64.gnu.opt.cpx 1 $dir $n $T $p $demand $sym $bin $intra $id $met $intra
```
in exec.sh uses the following arguments :

- dir : directory containing the instance to solve
- Parameters "$n $T $p $demand $sym $bin $intra $id" refer to the instance characteristics as presented above
- met : unique number referring to a particular tuple of parameters to be used for the resolution (see main.cpp)
- intra : equals 1 if intra-site constraints must be taken into account

## Parameters

* **nonLinearStartUpCost**

If true: start-up cost are not constant anymore but are an increasing function of the down time (see function ComputeStartUpCosts in DynProgSUSDPricingAlgo.cpp)

Note: bug in master.cpp, start-up costs are not computed correctly in this case in function "computeCost" of class MasterVariable


