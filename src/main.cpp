#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "scip/dialog_default.h"

#include <ilcplex/ilocplex.h>

/* scip includes */
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

/* user defined includes */
#include "InstanceUCP.h"
#include "Process.h"
#include "Master.h"
#include "Pricer.h"
#include "BranchConsHandler.h"
#include "BranchingRule.h"
#include "CplexPricingAlgo.h"
#include "Checker.h"
#include "IUPHandler.h"

/* namespace usage */
using namespace std;
using namespace scip;

//#define SCIP_OUTPUT

#define SCIP_DEBUG

int main(int argc, char** argv)
{
    ofstream fichier("result.txt", std::ofstream::out | std::ofstream::app);


    //////////////////////////////
    //////  INSTANCE DATA    /////
    //////////////////////////////

    int T ;
    int n  ;
    int sym  ;
    int demande ;
    int cat01 ;
    int bloc ;
    int intra  ;
    int id;
    int met = 0 ;
    string localisation;

    if (argc==1) { // pour debug
        //Paramètres de l'instance
        T = 24;
        n = 20;
        sym = 3;
        demande = 4;
        cat01 = 0;
        bloc = 1;
        intra = 1 ;
        id=1 ;
        localisation = "data/sym=site/" ;
    }
    if (argc>1) {
        localisation = argv[2] ;
        n = atoi(argv[3]);
        T = atoi(argv[4]);
        bloc = atoi(argv[5]);
        demande = atoi(argv[6]);
        sym = atoi(argv[7]);
        cat01 = atoi(argv[8]);
        intra = atoi(argv[9]);
        id = atoi(argv[10]);
        met = atoi(argv[11]);
    }

    InstanceProcessed Instance = InstanceProcessed(n, T, bloc, demande, sym, cat01, intra, id, localisation) ;

    string nom = Instance.fileName() ;
    const char* file = strdup(nom.c_str()) ;

    IloEnv env;
    InstanceUCP* inst = new InstanceUCP(env, file) ;


    ///////////////////////////
    //////  PARAMETERS    /////
    ///////////////////////////

    bool IP=0 ; // est-ce qu'on résout le master en variable entières ?
    bool ManageSubPbSym=0 ; // est-ce qu'on gère les symétries dans le sous problème ?
    bool Ramp=0 ; // est-ce qu'on considère les gradients ?
    bool TimeStepDec = 1 ;
    bool IntraSite = 0 ; // à implémenter
    bool DemandeResiduelle = 0 ;
    bool Iup = 0 ;
    double eps = 0.0000001;
    bool heuristicInit = 0 ;
    bool DontPriceAllTimeSteps = 1;
    bool DontGetPValue = 0 ;
    bool OneTimeStepPerIter = 0;

    cout << "met: " << met << endl ;
    if (met==1) {
        DontPriceAllTimeSteps=true ;
    }

    if (met==2) {
        DontPriceAllTimeSteps=true ;
        DontGetPValue = true ;
    }

    Parameters const param(IP, ManageSubPbSym, Ramp, TimeStepDec, IntraSite, DemandeResiduelle, Iup, eps, DontPriceAllTimeSteps, heuristicInit, DontGetPValue, OneTimeStepPerIter);


    ////////////////////////////////////
    //////  SCIP INITIALIZATION    /////
    ////////////////////////////////////

    clock_t start;
    start = clock();

    // problem initialization
    SCIP *scip=NULL;
    SCIPcreate(&scip);

    SCIPprintVersion(scip, NULL);
    SCIPinfoMessage(scip, NULL, "\n");

    /* include default plugins */
    //SCIPincludeDefaultPlugins(scip);


    SCIPincludeConshdlrLinear(scip);
    SCIPincludeNodeselBfs(scip);
    SCIPincludeConshdlrIntegral(scip);
    SCIPincludeDispDefault(scip);
    SCIPincludeDialogDefault(scip);
    SCIPincludeHeurActconsdiving(scip);
    SCIPincludeHeurClique(scip);
    SCIPincludeHeurCoefdiving(scip);
    SCIPincludeHeurCrossover(scip);
    SCIPincludeHeurDins(scip);
    SCIPincludeHeurFeaspump(scip);
    SCIPincludeHeurFixandinfer(scip);
    SCIPincludeHeurFracdiving(scip);
    SCIPincludeHeurGuideddiving(scip);
    SCIPincludeHeurIntdiving(scip);
    SCIPincludeHeurIntshifting(scip);
    SCIPincludeHeurLinesearchdiving(scip);
    SCIPincludeHeurLocalbranching(scip);
    SCIPincludeHeurMutation(scip);
    SCIPincludeHeurObjpscostdiving(scip);
    SCIPincludeHeurOctane(scip);
    SCIPincludeHeurOneopt(scip);
    SCIPincludeHeurPscostdiving(scip);
    SCIPincludeHeurRens(scip);
    SCIPincludeHeurRins(scip);
    SCIPincludeHeurShiftandpropagate(scip);
    SCIPincludeHeurShifting(scip);
    SCIPincludeHeurSimplerounding(scip);
    SCIPincludeHeurSubNlp(scip);
    SCIPincludeHeurTrivial(scip);
    SCIPincludeHeurTrySol(scip);
    SCIPincludeHeurTwoopt(scip);
    SCIPincludeHeurUndercover(scip);
    SCIPincludeHeurVbounds(scip);
    SCIPincludeHeurVeclendiving(scip);
    SCIPincludeHeurZirounding(scip);
    SCIPincludeHeurRootsoldiving(scip);
    SCIPincludeHeurRounding(scip);

    //SCIPsetLongintParam(scip, "limits/nodes", 12);
    SCIPsetRealParam(scip, "limits/time", 3600);

    SCIPincludeDispDefault(scip) ;
    SCIPincludeDialogDefault(scip) ;

    /* set verbosity parameter */
    SCIPsetIntParam(scip, "display/verblevel", 5);
    //SCIPsetBoolParam(scip, "display/lpinfo", TRUE);

    /* create empty problem */
    SCIPcreateProb(scip, "UCP", 0, 0, 0, 0, 0, 0, 0);



    //////////////////////////////////////////////
    //////  MASTER PROBLEM INITIALIZATION    /////
    //////////////////////////////////////////////


    // Initialisation du checker
    CplexChecker checker = CplexChecker(inst, param) ;

    //Master problem
    Master_Model Master(inst, param) ;
    MasterTime_Model MasterTime(inst, param) ;


    if (param.TimeStepDec) { // décomposition par pas de temps
        MasterTime.initScipMasterTimeModel(scip);
    }

    else { // décomposition classique
        Master.InitScipMasterModel(scip, inst) ;
    }


    if (param.heuristicInit) {
        IloNumArray x(env, n*T) ;
        IloNumArray p(env, n*T) ;
        checker.CplexPrimalHeuristic(x,p);
        MasterTime.createColumns(scip, x,p);
    }

    ////////////////////////////////
    //////  PRICING PROBLEM    /////
    ////////////////////////////////

    static const char* PRICER_NAME = "Pricer_UCP";

    // include UCP pricer

    ObjPricerTimeUCP* pricerTime ;
    ObjPricerUCP* pricer ;

    if (param.TimeStepDec) {
        pricerTime = new ObjPricerTimeUCP(scip, PRICER_NAME, &MasterTime, inst, param);
        SCIPincludeObjPricer(scip, pricerTime, true);
        SCIPactivatePricer(scip, SCIPfindPricer(scip, PRICER_NAME));

    }
    else {
        pricer = new ObjPricerUCP(scip, PRICER_NAME, &Master, inst, param);
        SCIPincludeObjPricer(scip, pricer, true);
        SCIPactivatePricer(scip, SCIPfindPricer(scip, PRICER_NAME));
    }

    cout<<"Write init pl"<<endl;
    SCIPwriteOrigProblem(scip, "init.lp", "lp", FALSE);



    //////////////////////////////////
    /////  VALID INEQUALITIES    /////
    //////////////////////////////////

    if (param.IntervalUpSet && param.TimeStepDec) {
        IUPHandler* iupHandler = new IUPHandler(scip, &MasterTime, inst);
        SCIPincludeObjConshdlr(scip, iupHandler, TRUE);
    }

    /////////////////////////
    /////  BRANCHING    /////
    /////////////////////////

    if (param.IP) {
        //        BranchConsHandler* branchConsHandler = new BranchConsHandler(scip, pricer_ptr);
        //        BranchingRule* branchRule = new BranchingRule(scip, inst,  &Master, pricer_ptr);

        //        SCIPincludeObjConshdlr(scip, branchConsHandler, TRUE);
        //        SCIPincludeObjBranchrule(scip, branchRule, TRUE);
    }

    //////////////////////
    //////  SOLVE    /////
    //////////////////////

    SCIPsolve(scip);

    double temps_scip = ( clock() - start ) / (double) CLOCKS_PER_SEC;
    /// Solution en x

    n=inst->getn();
    T=inst->getT();

    vector<double> x_frac = vector<double>(n*T, 0) ;

    if (param.TimeStepDec) {
        list<MasterTime_Variable*>::const_iterator itv;
        SCIP_Real frac_value;


        for (itv = MasterTime.L_var.begin(); itv!=MasterTime.L_var.end(); itv++) {

            frac_value = fabs(SCIPgetVarSol(scip,(*itv)->ptr));

            int time = (*itv)->time ;
            for (int i=0 ; i < n ; i++) {

                    if ((*itv)->UpDown_plan[i] > param.Epsilon) {

                        x_frac[i*T+time] += frac_value ;
                    }

            }
        }
        cout << "solution x frac: " << endl;

        for (int t=0 ; t < T ; t++) {
            for (int i=0 ; i <n ; i++) {
                cout << x_frac[i*T+t] << " " ;
            }
            cout << endl ;
        }
    }


    else {
        list<Master_Variable*>::const_iterator itv;
        SCIP_Real frac_value;

        for (itv = Master.L_var.begin(); itv!=Master.L_var.end(); itv++) {

            frac_value = fabs(SCIPgetVarSol(scip,(*itv)->ptr));

            int site = (*itv)->Site ;
            int first = inst->firstUnit(site) ;
            for (int i=0 ; i < inst->nbUnits(site) ; i++) {
                for (int t=0 ; t < T ; t++) {

                    if ((*itv)->UpDown_plan[i*T+t] > eps) {

                        x_frac[(first+i)*T+t] += frac_value ;
                    }
                }
            }
        }


        cout << "solution x frac: " << endl;

        for (int t=0 ; t < T ; t++) {
            for (int i=0 ; i <n ; i++) {
                cout << x_frac[i*T+t] << " " ;
            }
            cout << endl ;
        }
    }



    //////////////////////
    //////   STATS   /////
    //////////////////////

    SCIP_PRICER ** scippricer = SCIPgetPricers(scip);

    fichier << n << " & " << T << " & " << id ;
    fichier << " &  " << SCIPgetNNodes(scip) ;
    fichier << " & " << SCIPgetNPricevarsFound(scip) ;
    //fichier << " & " << SCIPpricerGetTime(scippricer[0]) ;
    if (param.TimeStepDec) {
        fichier << " & " << pricerTime->nbCallsToCplex ;
        fichier << " & " << MasterTime.cumul_resolution_pricing ;
    }
    //fichier << " &  " << SCIPgetSolvingTime(scip) ;
    fichier << " & " << temps_scip  ;
    fichier << " &  " << SCIPgetNLPIterations(scip) ;
    fichier << " &  " << SCIPgetGap(scip);
    fichier << " &  " << SCIPgetPrimalbound(scip);
    fichier << " &  " << SCIPgetDualbound(scip);

    cout << "ici" << endl ;

    //////////////////////
    //////  CHECK    /////
    //////////////////////

    cout.precision(15);

    //    vector<double> x_frac = vector<double>(n*T, 0) ;
    //    for (int i=0 ; i < n*T ; i++) {
    //        x_frac[i] = (checker.cplex).getValue(checker.x[i], x_frac[i]) ;
    //    }

    fichier << "& " << checker.getLRValue() ; // RL
    fichier << "& " << checker.getLRCplex() ; // RL CPLEX
    //fichier << " & " << checker.getIntegerObjValue(); // OPT
    fichier <<" \\\\ " << endl ;


    //    cout << "check x_frac: " << endl ;
    checker.checkSolution(x_frac);

    return 0;
}

