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
#include "IntervalUpSet.h"


/* namespace usage */
using namespace std;
using namespace scip;

/////////////////////////////////////
////////// INTERVAL UP SET //////////
/////////////////////////////////////


class IneqIntUpSet {
public:
    list<int>* C ;
    int alpha ;
    int i ;
    int t0 ;
    int t1 ;

    SCIP_CONS* ineq ;
    double dual ;

    IneqIntUpSet(SCIP* scip, int num, int alpha, list<int>* C_ptr, int i, int t0, int t1) ;
};

//////////////////////////////////////////////
////////// DECOMPOSITION PAR UNITES //////////
//////////////////////////////////////////////


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
    void computeCost(InstanceUCP* inst, const Parameters & Param) ;

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

    InstanceUCP* inst ;
    double Relax_withoutIUP ;

    double cumul_resolution_pricing ;
    const Parameters Param ;


    IloEnv env;

    //Variables u
    vector<SCIP_VAR*> u_var ;

    // Keep a pointer on every constraint of the MasterTime program (except intrasite constraints which do not depend on lambda variables)
    vector<SCIP_CONS*> logical;
    vector<SCIP_CONS*> min_up;
    vector<SCIP_CONS*> min_down;
    vector<SCIP_CONS*> convexity_cstr;

    //Interval up set inequalities
    int nbIntUpSet ;
    vector< list<IneqIntUpSet*> > IUP_t0 ; // IUP_t0[t] : liste des interval-up-set telles que t0=t
    vector< list<IneqIntUpSet*> > IUP_t1 ; // IUP_t1[t] : liste des interval-up-set telles que t1=t. Redondant mais plus efficace

    // Keep informations on every variables of the Master program
    //NB: le fait d'utiliser une liste ne permet pas de supprimer des variables
    list<MasterTime_Variable*> L_var;

    MasterTime_Model(InstanceUCP* inst, const Parameters & Param) ;

    void addCoefsToConstraints(SCIP* scip, MasterTime_Variable* lambda) ;

    void initScipMasterTimeModel(SCIP* scip);
    void initMasterTimeVariable(SCIP* scip, MasterTime_Variable* lambda) ;

    void createColumns(SCIP* scip, IloNumArray x, IloNumArray p) ;

    void addIntUpSet(SCIP* scip, IneqIntUpSet* Iup) ;// ajoute les coef necessaires dans l'inégalité, ajoute l'inégalité à SCIP, met à jour les vecteurs IUP_t0 et IUP_t1


};
#endif /* MASTER INCLUDED */
