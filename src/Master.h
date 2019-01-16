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
////////// BRANCHING CONS DATA //////
/////////////////////////////////////


// Data associated to a constraint (each artificial constraint represents one branching constraint)
struct SCIP_ConsData {
    int VarX ; // =1 si la variable branchée est un x. Si branchement sur u: VarX=0
    int bound ; // variable fixée à 1 ou à 0
    int unit; // which unit of the site
    int time ;
    int site ;
    IloRange BranchConstraint ;
    list<SCIP_VAR*> L_var_bound;
};



/////////////////////////////////////
////////// INTERVAL UP SET //////////
/////////////////////////////////////

class IneqIntUpSet {
public:
    list<int>* C ; // liste des éléments de C privés de i (liste renvoyée par les fonctions de séparation dans IntervalUpSet.cpp)
    int alpha ;
    int i ;
    int t0 ;
    int t1 ;

    SCIP_CONS* ineq ;
    double dual ;

    IneqIntUpSet(SCIP* scip, const Parameters & Parametres, int num, int alpha, list<int>* C_ptr, int i, int t0, int t1) ;
};

////////////////////////////////////////////
////////// MASTER MODEL (virtual) //////////
////////////////////////////////////////////

class Master_Model {
public:
    int n ;
    int T ;

    const Parameters Param ;
    InstanceUCP* inst ;

    list<double> totalDualCostList;

    int nbIntUpSet ;

    //Interval up set inequalities
    vector< list<IneqIntUpSet*> > IUP_t0 ; // IUP_t0[t] : liste des interval-up-set telles que t0=t
    vector< list<IneqIntUpSet*> > IUP_t1 ; // IUP_t1[t] : liste des interval-up-set telles que t1=t. Redondant mais plus efficace

    vector<double> x_frac ;
    double cumul_resolution_pricing ;

    Master_Model(const Parameters & Par, InstanceUCP* i) : Param(Par), inst(i) {
        n= inst->getn();
        T= inst->getT() ;
        nbIntUpSet = 0 ;
        x_frac.resize(n*T,0);

        IUP_t0.resize(T) ;
        IUP_t1.resize(T) ;

        for (int t=0 ; t < T ; t++) {
            (IUP_t0.at(t)).clear() ;
            (IUP_t1.at(t)).clear() ;
        }
    }
    virtual void computeFracSol(SCIP* scip) = 0;  // = 0 signifie "virtuelle pure"

    virtual void discardVar(SCIP* scip, SCIP_ConsData* consdata) = 0;
    virtual void restoreVar(SCIP* scip, SCIP_ConsData* consdata) = 0;

    virtual ~Master_Model() {}
};

///////////////////////////////
////////// VARIABLES //////////
///////////////////////////////


class Master_Variable{
public:

    /// Keep a pointer on every variable of the Master program
    SCIP_VAR* ptr;

    /// site corresponding to variable ptr
    int Site ; //index of the site

    //// up/down plan corresponding to ptr
    IloNumArray UpDown_plan ;

    //// power production plan (if given by master variable lambda)
    IloNumArray Power_plan ;

    double cost ;

    Master_Variable(int site, IloNumArray UpDown) ;
    void addPowerPlan(IloNumArray PowerPlan);
    void computeCost(InstanceUCP* inst, const Parameters & Param) ;

};



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


////////////////////////////////////////////////////
////////// MASTER -- DECOMPOSITION PAR SITES ///////
////////////////////////////////////////////////////


class MasterSite_Model : public Master_Model {
public:

    int S ;
    IloEnv env;

    // Keep a pointer on every constraint of the Master program
    vector<SCIP_CONS*> demand_cstr;
    vector<SCIP_CONS*> power_limits;
    vector<SCIP_CONS*> ramp_up;
    vector<SCIP_CONS*> ramp_down;
    vector<SCIP_CONS*> convexity_cstr;
    vector<SCIP_CONS*> intrasite;

    vector<SCIP_CONS*> mindown; //start up decomposition only. contrainte minup en fait
    vector<SCIP_CONS*> z_lambda; //start up decomposition only
    vector<SCIP_CONS*> logical; //start up decomposition only

    // Keep informations on every variables of the Master program
    //NB: le fait d'utiliser une liste ne permet pas de supprimer des variables
    list<Master_Variable*> L_var;

    MasterSite_Model(InstanceUCP* inst, const Parameters & Param) ;

    void addCoefsToConstraints(SCIP* scip, Master_Variable* lambda, InstanceUCP* inst) ;
    void  InitScipMasterModel(SCIP* scip, InstanceUCP* inst);
    void initMasterVariable(SCIP* scip, InstanceUCP* inst , Master_Variable* lambda) ;
    void createColumns(SCIP* scip, IloNumArray x) ;

    void computeFracSol(SCIP* scip) ;

    void discardVar(SCIP* scip, SCIP_ConsData* consdata) ;
    void restoreVar(SCIP* scip, SCIP_ConsData* consdata) ;
};


////////////////////////////////////////////////////
////////// DECOMPOSITION PAR PAS DE TEMPS //////////
////////////////////////////////////////////////////


class MasterTime_Model : public Master_Model {
public:



    IloEnv env;

    // Keep a pointer on every constraint of the MasterTime program (except intrasite constraints which do not depend on lambda variables)
    vector<SCIP_CONS*> logical;
    vector<SCIP_CONS*> min_up;
    vector<SCIP_CONS*> min_down;
    vector<SCIP_CONS*> convexity_cstr;
    vector<SCIP_CONS*> intrasite;

    vector<SCIP_CONS*> rsu;

    vector<SCIP_CONS*> rsd;



    // Keep informations on every variables of the Master program
    //NB: le fait d'utiliser une liste ne permet pas de supprimer des variables
    list<MasterTime_Variable*> L_var;

    //Variables u
    vector<SCIP_VAR*> u_var ;

    MasterTime_Model(InstanceUCP* inst, const Parameters & Param) ;

    void addCoefsToConstraints(SCIP* scip, MasterTime_Variable* lambda) ;

    void initScipMasterTimeModel(SCIP* scip);
    void initMasterTimeVariable(SCIP* scip, MasterTime_Variable* lambda) ;

    void createColumns(SCIP* scip, IloNumArray x, IloNumArray p) ;

    void addIntUpSet(SCIP* scip, IneqIntUpSet* Iup) ;// ajoute les coef necessaires dans l'inégalité, ajoute l'inégalité à SCIP, met à jour les vecteurs IUP_t0 et IUP_t1

    void computeFracSol(SCIP* scip) ;

    void discardVar(SCIP* scip, SCIP_ConsData* consdata) ;
    void restoreVar(SCIP* scip, SCIP_ConsData* consdata) ;

};


////////////////////////////////////////////////////
////////// MASTER -- DOUBLE DECOMPOSITION ///////
////////////////////////////////////////////////////


class MasterDouble_Model : public Master_Model {
public:

    int S ;
    IloEnv env;

    // Keep a pointer on every constraint of the Master program

    vector<SCIP_CONS*> conv_lambda_site;
    vector<SCIP_CONS*> conv_lambda_time;
    vector<SCIP_CONS*> eq_time_site;

    //si option de stabilisation minUpDownDouble=1
    vector<SCIP_CONS*> logical;
    vector<SCIP_CONS*> min_up;
    vector<SCIP_CONS*> min_down;

    // Keep informations on every variables of the Master program
    //NB: le fait d'utiliser une liste ne permet pas de supprimer des variables
    list<Master_Variable*> L_var_site;
    list<MasterTime_Variable*> L_var_time;

    MasterDouble_Model(InstanceUCP* inst, const Parameters & Param) ;

    void addCoefsToConstraints_siteVar(SCIP* scip, Master_Variable* lambda, InstanceUCP* inst) ;

    void addCoefsToConstraints_timeVar(SCIP* scip, MasterTime_Variable* lambda) ;

    void initScipMasterDoubleModel(SCIP* scip, InstanceUCP* inst);
    void initMasterSiteVariable(SCIP* scip, InstanceUCP* inst , Master_Variable* lambda) ;
    void initMasterTimeVariable(SCIP* scip, MasterTime_Variable* lambda) ;
    void createColumns(SCIP* scip, IloNumArray x, IloNumArray p) ;

    void computeFracSol(SCIP* scip) ;

    void discardVar(SCIP* scip, SCIP_ConsData* consdata) ;
    void restoreVar(SCIP* scip, SCIP_ConsData* consdata) ;
};
#endif /* MASTER INCLUDED */
