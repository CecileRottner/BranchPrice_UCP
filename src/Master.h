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
    IloNumArray UpDown_plan ;

    double cost ;

    Master_Variable(int site, IloNumArray UpDown) ;
    void computeCost(InstanceUCP* inst) ;

};

class Master_Model{
public:
    int n ;
    int T ;
    int S ;


    const Parameters Param ;

    IloEnv env;

    // Keep a pointer on every constraint of the Master program
    vector<SCIP_CONS*> demand_cstr;
    vector<SCIP_CONS*> power_limits;
    vector<SCIP_CONS*> ramp_up;
    vector<SCIP_CONS*> ramp_down;
    vector<SCIP_CONS*> convexity_cstr;

    // Keep informations on every variables of the Master program
    //NB: le fait d'utiliser une liste ne permet pas de supprimer des variables
    list<Master_Variable*> L_var;

    Master_Model(InstanceUCP* inst, const Parameters & Param) ;

    void addCoefsToConstraints(SCIP* scip, Master_Variable* lambda, InstanceUCP* inst) ;
    void computeCost(InstanceUCP* inst) ;

    void  InitScipMasterModel(SCIP* scip, InstanceUCP* inst);
    void initMasterVariable(SCIP* scip, InstanceUCP* inst , Master_Variable* lambda) ;


};


////////////////////////////////////////////////////
////////// DECOMPOSITION PAR PAS DE TEMPS //////////
////////////////////////////////////////////////////

class MasterTime_Variable{
public:

    /// Keep a pointer on every variable of the Master program
    SCIP_VAR* ptr;

    /// time period corresponding to variable ptr
    int time ;

    //// up/down plan corresponding to ptr
    IloNumArray UpDown_plan ;

    double cost ;

    MasterTime_Variable(int site, IloNumArray UpDown, double costFromSubPb) ;

};


class MasterTime_Model{
public:
    int n ;
    int T ;
    int S ;

    int cumul_resolution_pricing ;
    const Parameters Param ;

    IloEnv env;

    // Keep a pointer on every constraint of the MasterTime program (except intrasite constraints which do not depend on lambda variables)
    vector<SCIP_CONS*> logical;
    vector<SCIP_CONS*> min_up;
    vector<SCIP_CONS*> min_down;
    vector<SCIP_CONS*> convexity_cstr;

    // Keep informations on every variables of the Master program
    //NB: le fait d'utiliser une liste ne permet pas de supprimer des variables
    list<MasterTime_Variable*> L_var;

    MasterTime_Model(InstanceUCP* inst, const Parameters & Param) ;

    void addCoefsToConstraints(SCIP* scip, MasterTime_Variable* lambda, InstanceUCP* inst) ;

    void  InitScipMasterTimeModel(SCIP* scip, InstanceUCP* inst);
    void initMasterTimeVariable(SCIP* scip, InstanceUCP* inst , MasterTime_Variable* lambda) ;


};
#endif /* MASTER INCLUDED */
