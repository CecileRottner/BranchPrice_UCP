# Branch&Price for the Unit Commitment Problem

Runs with SCIP 5.0.1 and Cplex

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
- intra : equals 1 if instances are subject to "intra site" constraints (ie units are grouped on geopgraphical sites and there is at most one start up per time period on each site)
- id : unique identifier of the instance

## Arguments

Command line 
```bash
./bin/SCIP_UCP_BP.linux.x86_64.gnu.opt.cpx 1 $dir $n $T $p $demand $sym $cat01 $intra $id $met $UseIntraCons
```
in exec.sh uses the following arguments :

- dir : directory containing the data file to use
