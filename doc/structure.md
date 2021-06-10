# Main

- Reads instance

- Based on indicator "met" given as argument, builds Parameters class

- Initializes an empty SCIP problem

- Initializes Master and Pricer problems via virtual classes Master_Model and ObjPricerUCP (inherited from SCIP pricer class ObjPricer) :

Depending on the decomposition chosen (unit, time, overlapping...), classes implementing master and pricing problems will be constructed as specific classes inheriting from Master_Model and ObjPricerUCP.
  - for a unit/site decomposition, we use classes MasterSite_Model and ObjPricerSite
  - for a time decomposition, we use classes MasterTime_Model and ObjPricerTimeUCP
  - for an overlapping unit/site and time decomposition, we use classes MasterDouble_Model and ObjPricerDouble
  
  For each decomposition type, initial columns are defined using Cplex primal heuristics at root node.
  
- Initializes branching rules via classes BranchConsHandler and BranchingRules

- Solves the problem

- Prints/exports solution and resolution indicators
