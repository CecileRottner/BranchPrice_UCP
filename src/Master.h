#ifndef MASTER
#define MASTER

#include <ilcplex/ilocplex.h>

#include <vector>
#include <list>

/* scip includes */
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

/* user defined includes */
#include "InstanceUCP.h"
#include "Process.h"


/* namespace usage */
using namespace std;
using namespace scip;



class Master_Variable{
public:

    /// Keep a pointer on every variable of the Master program
    SCIP_VAR* ptr;

    /// site corresponding to variable ptr
    int Site ; //index of the site

    //// up/down plan corresponding to ptr
    IloIntArray UpDown_plan ;

    double cost ;

    Master_Variable(int site, IloIntArray UpDown) ;
    void computeCost(InstanceUCP* inst) ;

};

class Master_Model{
public:
    int n ;
    int T ;
    int S ;

    IloEnv env;

    // Keep a pointer on every constraint of the Master program
    vector<SCIP_CONS*> demand_cstr;
    vector<SCIP_CONS*> power_limits;
    vector<SCIP_CONS*> convexity_cstr;

    // Keep informations on every variables of the Master program
    //NB: le fait d'utiliser une liste ne permet pas de supprimer des variables
    list<Master_Variable*> L_var;

    Master_Model(InstanceUCP* inst) ;
    void  InitScipMasterModel(SCIP* scip, InstanceUCP* inst);

};

#endif /* MASTER INCLUDED */
