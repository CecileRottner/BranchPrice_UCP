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
    int intra_cons;
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
        intra_cons = atoi(argv[12]) ;
    }

    InstanceProcessed Instance = InstanceProcessed(n, T, bloc, demande, sym, cat01, intra, id, localisation) ;

    string nom = Instance.fileName() ;
    const char* file = strdup(nom.c_str()) ;

    IloEnv env;
    InstanceUCP* inst = new InstanceUCP(env, file) ;


    ///////////////////////////
    //////  PARAMETERS    /////
    ///////////////////////////

    double eps = 0.0000001; // tolérance
    int node_limit =1000000000;

    bool IP=1; // est-ce qu'on résout le master en variable entières ?
    bool PriceAndBranch = 0;

    bool IntraSite = intra_cons ; // contraintes intrasites prises en compte ou pas


    //// Paramètres: type de décomposition ////
    bool UnitDecompo = false ;
    bool StartUpDecompo = false;


    bool TimeStepDec = 0 ;
    bool DynProgTime = 0 ; // implémenté pour Pmax=Pmin et décomposition par pas de temps
    bool DynProg = 0; // implémenté pour Pmax=Pmin et décomposition par unités

    bool DemandeResiduelle = 0 ;

    bool doubleDecompo = 0 ;

    /////////////////////////////////////////



    //// Paramètres symétries et interval up-set ///
    bool IntervalUpSet = 0 ;// implémenté pour time decomposition seulement (résolution dyn prog)
    bool masterSSBI = 0 ; // implémenté pour time decomposition seulement (résolution dyn prog)

    bool useSSBIinSubPb = false;
    bool ManageSubPbSym = 0 ; // est-ce qu'on gère les symétries dans le sous problème ? --> paramètre inutile. à enlever

    bool heuristicInit = 0;
    ////////////////////////////////////


    //// paramètres gradients /////
    /// implémentés pour unit decomposition
    bool Ramp = 0 ; // gradients pris en compte ou pas
    bool RampInMaster = 0 ; // contraintes de gradients dualisées
    bool RampInSubPb = 0 ; // contraintes de gradient non dualisées

    bool powerPlanGivenByLambda = false ; // implémenté pour unit et site décompo seulement. Les puissances sont uniquement dans le sous problème. utile pour les ramp.
                                          // pas compatible avec la décomposition start up notamment
    //////////////////////////////


    ///// Paramètres pricing décomposition par pas de temps. //////
    bool DontPriceAllTimeSteps = 0;
    bool DontGetPValue = 0 ;
    bool OneTimeStepPerIter = 0;
    bool addColumnToOtherTimeSteps = 0 ;
    ////////////////////////////////////////////////////////////////

    bool Solve = true ; // true: Branch & Price avec SCIP. false: résolution Cplex boîte noire


    if (met==-1) {
        Solve = false ;
    }


    //// COMPARISONS
    ///
    /// Unit subset decompositions
    ///

    if (met == 100) { // UNIT DECOMPOSITION ---- BRANCH AND PRICE
        UnitDecompo=true;
        heuristicInit=1 ;
        DynProg=1 ;

    }

    if (met == 101) { // UNIT DECOMPOSITION ---- COL GEN
        UnitDecompo=true;
        DynProg=1 ;
        node_limit=1 ;

    }

    if (met == 1012) { // UNIT DECOMPOSITION ---- COL GEN, subpb résolu par Cplex
        UnitDecompo=true;
        node_limit=1 ;
        RampInMaster=1 ;

    }

    if (met == 1011) { // UNIT START UP DECOMPOSITION ---- COL GEN
        UnitDecompo=true;
        StartUpDecompo=true;

        node_limit=1 ;
    }

    if (met == 102) { // SITE DECOMPOSITION ---- COL GEN

        node_limit=1 ;
    }
    if (met == 1022) { // SITE DECOMPOSITION WITH SSBI in SUBPB ---- COL GEN
        useSSBIinSubPb=true;
        node_limit=1 ;
    }

    if (met == 1021) { // SITE START-UP DECOMPOSITION ---- COL GEN
        StartUpDecompo=true;

        node_limit=1 ;
    }

    if (met == 103) { // RESIDUAL DEMAND DECOMPOSITION ---- COL GEN
        DemandeResiduelle= true;

        node_limit=1 ;
    }
    if (met == 1031) { // RESIDUAL DEMAND START UP DECOMPOSITION ---- COL GEN
        DemandeResiduelle= true;
        StartUpDecompo=true;

        node_limit=1 ;
    }


    if (met == 104) { // UNIT DECOMPOSITION with P GIVEN BY LAMBDA ---- COL GEN
        UnitDecompo=true;
        powerPlanGivenByLambda = true;
        node_limit=1 ;
        RampInSubPb=1 ;

    }

    /// Time decomposition

    if (met == 200) {
        TimeStepDec = true ;
        DynProgTime = true ;
    }

    if (met == 201) {
        TimeStepDec = true ;
        DynProgTime = true ;
        node_limit=1 ;
    }

    if (met == 2011) {
        TimeStepDec = true ;
        DynProgTime = true ;
        IntraSite=0;
        node_limit=1 ;
    }

    if (met == 202) {
        TimeStepDec = true ;
        DynProgTime = true ;
        IntervalUpSet = true;

        node_limit=1 ;
    }



    if (met == 203) {
        TimeStepDec = true ;
        DynProgTime = true ;
        node_limit=1 ;
        masterSSBI=1 ;
    }
    if (met == 204) {
        TimeStepDec = true ;
        DynProgTime = true ;
        masterSSBI=1 ;
    }

    if (met== 301) {
        doubleDecompo =true ;
        node_limit=1 ;

        IntraSite=0 ;

        UnitDecompo=true;
        DynProg=1 ;

        DynProgTime=true ;
    }

    /// Branch & Price (& Cut)
    if (met==1) {
        heuristicInit=1 ;
    }
    if (met==2) {
        heuristicInit=1 ;
        IntervalUpSet =1 ;
    }

    /// Price and Branch
    if (met==3) {
        PriceAndBranch=1;
        heuristicInit=1;
    }
    if (met==4) {
        PriceAndBranch=1;
        heuristicInit=1;
        IntervalUpSet = 1 ;
        Ramp = 1 ;
    }

    if (met==10) {
        TimeStepDec = 0;
        DynProgTime = 0 ;
    }

    if (met==11) {
        TimeStepDec = 0;
        DynProgTime = 0 ;
    }



    Parameters const param(inst, IP, ManageSubPbSym, Ramp, TimeStepDec, IntraSite, DemandeResiduelle, IntervalUpSet, eps, DontPriceAllTimeSteps,
                           heuristicInit, DontGetPValue, OneTimeStepPerIter, addColumnToOtherTimeSteps, DynProgTime, DynProg, PriceAndBranch,
                           UnitDecompo, StartUpDecompo, useSSBIinSubPb, powerPlanGivenByLambda, RampInMaster, RampInSubPb, masterSSBI, doubleDecompo);

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
   // SCIPincludeDefaultPlugins(scip);


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

    SCIPsetLongintParam(scip, "limits/nodes", node_limit);
    SCIPsetRealParam(scip, "limits/time", 3600);

    SCIPincludeDispDefault(scip) ;
    SCIPincludeDialogDefault(scip) ;

    /* set verbosity parameter */
    SCIPsetIntParam(scip, "display/verblevel", 5);
    //SCIPsetBoolParam(scip, "display/lpinfo", TRUE);

    /* create empty problem */
    SCIPcreateProb(scip, "UCP", 0, 0, 0, 0, 0, 0, 0);

    ////////////////////////////////////////////////////////
    //////  MASTER & PRICER PROBLEMS INITIALIZATION    /////
    ////////////////////////////////////////////////////////

    CplexChecker checker = CplexChecker(inst, param) ;

    Master_Model* Master_ptr;
    ObjPricerUCP* Pricer = NULL ;

    static const char* PRICER_NAME = "Pricer_UCP";

    if (param.TimeStepDec) { //// Décomposition par pas de temps
        Master_ptr = new MasterTime_Model(inst, param) ;


        MasterTime_Model* MT ;
        MT = dynamic_cast<MasterTime_Model*> (Master_ptr) ;

        if (MT != NULL) {


            ///Initialisation du master
            MT->initScipMasterTimeModel(scip);


            /// Initialisation du pricer
            Pricer = new ObjPricerTimeUCP(scip, PRICER_NAME, MT, inst, param);
            SCIPincludeObjPricer(scip, Pricer, true);
            SCIPactivatePricer(scip, SCIPfindPricer(scip, PRICER_NAME));
        }

        //// HEURISTIC INITIALIZATION OF MASTER'S COLUMNS
        if (param.heuristicInit) {
            IloNumArray x(env, n*T) ;
            IloNumArray p(env, n*T) ;
            checker.CplexPrimalHeuristic(x,p);
            MT->createColumns(scip, x,p);
        }

        //// INTERVAL UP SET ////
        if (param.IntervalUpSet) {
            IUPHandler* iupHandler = new IUPHandler(scip, MT, inst, param);
            SCIPincludeObjConshdlr(scip, iupHandler, TRUE);
        }
    }

    else if (param.doubleDecompo) {
        Master_ptr = new MasterDouble_Model(inst, param) ;
        MasterDouble_Model* MD ;
        MD = dynamic_cast<MasterDouble_Model*> (Master_ptr) ;

        if (MD != NULL) {


            ///Initialisation du master
            MD->initScipMasterDoubleModel(scip, inst);


            /// Initialisation du pricer
            Pricer = new ObjPricerDouble(scip, PRICER_NAME, MD, inst, param);
            SCIPincludeObjPricer(scip, Pricer, true);
            SCIPactivatePricer(scip, SCIPfindPricer(scip, PRICER_NAME));

            if (param.heuristicInit) {
                IloNumArray x(env, n*T) ;
                IloNumArray p(env, n*T) ;
                checker.CplexPrimalHeuristic(x,p);
                MD->createColumns(scip, x, p);
            }
        }



    }

    else { //// Décomposition par sites
        Master_ptr = new MasterSite_Model(inst, param) ;

        MasterSite_Model* MS ;
        MS = dynamic_cast<MasterSite_Model*> (Master_ptr) ;
        if (MS != NULL) {

            cout << "MS non NULL" << endl ;
            ///Initialisation du master
            MS->InitScipMasterModel(scip, inst) ;

            /// Initialisation du pricer
            Pricer = new ObjPricerSite(scip, PRICER_NAME, MS, inst, param);
            SCIPincludeObjPricer(scip, Pricer, true);
            SCIPactivatePricer(scip, SCIPfindPricer(scip, PRICER_NAME));
        }

        //// HEURISTIC INITIALIZATION OF MASTER'S COLUMNS
        if (param.heuristicInit) {
            IloNumArray x(env, n*T) ;
            IloNumArray p(env, n*T) ;
            checker.CplexPrimalHeuristic(x,p);
            MS->createColumns(scip, x);
        }
    }


    cout<<"Write init pl"<<endl;
    SCIPwriteOrigProblem(scip, "init.lp", "lp", FALSE);


    /////////////////////////
    /////  BRANCHING    /////
    /////////////////////////

    if (param.IP) {
        BranchConsHandler* branchConsHandler = new BranchConsHandler(scip, Master_ptr, Pricer);
        BranchingRule* branchRule = new BranchingRule(scip, inst,  Master_ptr, Pricer, param);

        SCIPincludeObjConshdlr(scip, branchConsHandler, TRUE);
        SCIPincludeObjBranchrule(scip, branchRule, TRUE);
    }

    //////////////////////
    //////  SOLVE    /////
    //////////////////////

    MasterSite_Model* MS ;
    MS = dynamic_cast<MasterSite_Model*> (Master_ptr) ;
    if (MS != NULL) {
        cout << MS->demand_cstr.size() << endl;
        cout << MS->convexity_cstr.size() << endl ;
    }


    if (Solve) {

        cout << "resolution..." << endl ;
        SCIPsolve(scip);
        SCIPwriteTransProblem(scip, "priced.lp", NULL, FALSE);
        cout << "fin resolution" << endl ;


        /// Solution en x
        n=inst->getn();
        T=inst->getT();
        Master_ptr->computeFracSol(scip);



//        cout << "solution x frac: " << endl;

//        for (int t=0 ; t < T ; t++) {
//            for (int i=0 ; i <n ; i++) {
//                cout << Master_ptr->x_frac[i*T+t] << " " ;
//            }
//            cout << endl ;
//        }

    }

    cout << "ici" << endl ;


    double temps_scip =  ( clock() - start ) / (double) CLOCKS_PER_SEC;
    //////////////////////
    //////   STATS   /////
    //////////////////////

    fichier.precision(7);

    cout << "ici write" << endl ;
    SCIP_PRICER ** scippricer = SCIPgetPricers(scip);

    if (met==101 || met==100) {
        fichier << " Unit " ;
    }
    if (met==1011) {
        fichier << " Unit-SU " ;
    }
    if (met==102) {
        fichier << "Site ";
    }
    if (met==1021) {
        fichier << "Site-SU ";
    }
    if (met==103) {
        fichier << "RD ";
    }
    if (met==201) {
        fichier << "Time ";
    }
    if (met==202) {
        fichier << "Time+I ";
    }
    //fichier << "met & n & T & id & nodes & IUP & Iter & Var & CPU & gap & RL & low & up & cplex heur \\\\ " << endl;
    fichier << " & " << n << " & " << T << " & " << demande << " & " << id ;



    ///// AFFICHAGE BRANCH AND PRICE
    if (Solve) {
        fichier << " &  " << SCIPgetNNodes(scip) ;
        if (param.IntervalUpSet) {
            fichier << " & " << Master_ptr->nbIntUpSet ;
        }
        else {
            fichier << " & - "  ;
        }
        fichier << " &  " << SCIPgetNLPIterations(scip) ;
        fichier << " & " << SCIPgetNPricevarsFound(scip) ;
        //fichier << " &  " << SCIPgetNNodes(scip) ;
        // fichier << " & " << Master_ptr->cumul_resolution_pricing ;

        //    if (param.TimeStepDec && !param.DynProgTime) {
        //        fichier << " & " << pricerTime->nbCallsToCplex ;
        //        fichier << " & " << MasterTime.cumul_resolution_pricing ;
        //    }

        double timeScip =  SCIPgetSolvingTime(scip) ;
        fichier << " &  " << timeScip;
        fichier << " & " <<  timeScip - SCIPpricerGetTime(scippricer[0]); // MASTER TIME
        //fichier << " & " << temps_scip  ;
        fichier << " &  " << SCIPgetGap(scip);
        // fichier << " &  " << SCIPgetDualboundRoot(scip) ;
        if (met==100) {
        fichier << " &  " << SCIPgetPrimalbound(scip) ;
        }
        else {
        fichier << " &  " << SCIPgetDualbound(scip) ;
        fichier << " &  " << SCIPgetPrimalbound(scip) ;
        }

       // fichier << " &  " << SCIPgetPrimalbound(scip) ;
        //   fichier << " & " << checker.valHeuristicCplex ; // OPT

//        if (n==10 && met==201) {
//            checker.getIntegerObjValue();
//            fichier << " & " << checker.PrimalBound ; // OPT
//        }

//        else {
//            fichier << " & - "  ; // OPT
//        }
                if (0 && (met*intra_cons==102) || (!intra_cons*met==103)) {

                    fichier << " & " << checker.getLRValue() ; // RL*/
                    fichier << " & " << checker.getLRCplex() ; // RL CPLEX

                    if (demande==4 || n < 20 || T < 48) {
                        checker.getIntegerObjValue();
                        fichier << " & " << checker.PrimalBound ; // OPT
                    }
                    else {
                        fichier << " & {-} "  ; // OPT
                    }

                }
                else{
                    fichier << " & & & " ;
                }
        fichier <<" \\\\ " << endl ;
        checker.checkSolution(Master_ptr->x_frac);

        cout << "ici fin write" << endl ;

    }


    else {


        //////////////////////
        //////  CHECK    /////
        //////////////////////

        cout.precision(15);

        //    vector<double> x_frac = vector<double>(n*T, 0) ;
        //    for (int i=0 ; i < n*T ; i++) {
        //        x_frac[i] = (checker.cplex).getValue(checker.x[i], x_frac[i]) ;
        //    }

        checker.getIntegerObjValue();

        fichier << " & " << checker.nbNodes ;
        fichier << " & - " ;
        fichier << " & - " ;
        fichier << " & - " ;
        fichier << " & " << checker.cpuTime ;
        fichier << " & " << checker.gap ;
        //   fichier << " & " << checker.getLRValue() ; // RL*/
        //  fichier << " & " << checker.getLRCplex() ; // RL CPLEX
        fichier << " & " << checker.DualBound ;
        fichier << " & " << checker.PrimalBound ; // OPT
        fichier << " & - " ;
        fichier <<" \\\\ " << endl ;
    }



//    ////// AFFICHAGE PRICE AND BRANCH
//    if (Solve) {
//        //fichier << " &  " << SCIPgetNNodes(scip) ;
//        fichier << " & - "  ;
//        if (param.IntervalUpSet) {
//            fichier << " & " << Master_ptr->nbIntUpSet ;
//        }
//        else {
//            fichier << " & - "  ;
//        }
////        fichier << " &  " << SCIPgetNLPIterations(scip) ;
//        fichier << " & " << SCIPgetNPricevarsFound(scip) ;
//        fichier << " & " << temps_scip ;
//        fichier << " &  " << SCIPgetDualbound(scip) ;

//       // fichier << " & " << Master_ptr->cumul_resolution_pricing ;

//        //    if (param.TimeStepDec && !param.DynProgTime) {
//        //        fichier << " & " << pricerTime->nbCallsToCplex ;
//        //        fichier << " & " << MasterTime.cumul_resolution_pricing ;
//        //    }

////        double timeScip =  SCIPgetSolvingTime(scip) ;
////        fichier << " &  " << timeScip;
////        fichier << " & " <<  timeScip - SCIPpricerGetTime(scippricer[0]); // MASTER TIME
////        //fichier << " & " << temps_scip  ;
////        fichier << " &  " << SCIPgetGap(scip);
////       // fichier << " &  " << SCIPgetDualboundRoot(scip) ;
////     //   fichier << " &  " << SCIPgetPrimalbound(scip) ;
////     //   fichier << " & " << checker.valHeuristicCplex ; // OPT

////        if (met*intra_cons==102 || !intra_cons*met==103) {

////            fichier << " & " << checker.getLRValue() ; // RL*/
////            fichier << " & " << checker.getLRCplex() ; // RL CPLEX

////            if (demande==4 || n < 20 || T < 48) {
////                checker.getIntegerObjValue();
////                fichier << " & " << checker.PrimalBound ; // OPT
////            }
////            else {
////                fichier << " & - "  ; // OPT
////            }

////        }
////        else{
////            fichier << " & & & " ;
////        }
////        fichier <<" \\\\ " << endl ;
//        //checker.checkSolution(Master_ptr->x_frac);

//    }


//    else {

//        //////////////////////
//        //////  CHECK    /////
//        //////////////////////

//        cout.precision(15);

//        //    vector<double> x_frac = vector<double>(n*T, 0) ;
//        //    for (int i=0 ; i < n*T ; i++) {
//        //        x_frac[i] = (checker.cplex).getValue(checker.x[i], x_frac[i]) ;
//        //    }

//        checker.getIntegerObjValue();

//        fichier << " & - " ;
//        fichier << " & - " ;
//        fichier << " & - "  ;
//        fichier << " & - "  ;

//        fichier << " & " << checker.DualBound ;

//        fichier << " & " << checker.PrimalBound ;
//         fichier << " & " << checker.cpuTime ;
//        fichier << " & " << checker.nbNodes ;
//        fichier << " & " << checker.gap << "\\%" ;
////        fichier << " & - " ;


////        fichier << " & " << checker.gap ;
////        fichier << " & " << checker.getLRValue() ; // RL*/
////        fichier << " & " << checker.getLRCplex() ; // RL CPLEX

////        // OPT
////        fichier << " & - " ;
//        fichier <<" \\\\ " << endl ;
//    }


    return 0;
}

