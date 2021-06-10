#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "scip/dialog_default.h"

#include <ilcplex/ilocplex.h>


#include <iomanip>
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
    ofstream fichierDualCost("dualCost.csv", std::ofstream::out | std::ofstream::app);


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

    Parameters param = init_parameters(inst, met, intra_cons);

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


    // include various SCIP features
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


    // resolution parameters (time and node limits)
    SCIPsetLongintParam(scip, "limits/nodes", param.nodeLimit);
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

    // frontal resolution with Cplex to verify results
    CplexChecker checker = CplexChecker(inst, param) ;

    // create master and pricing classes
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


    if (param.ColumnGeneration) {

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
    fichier << met << " & " << n << " & " << T << " & " << demande << " & " << id ;



    ///// AFFICHAGE BRANCH AND PRICE
    if (param.ColumnGeneration) {

        //checker.getIntegerObjValue();
        //checker.useLowBound(SCIPgetDualbound(scip));

        fichier << " &  " << SCIPgetNNodes(scip) ;
        if (param.IntervalUpSet) {
            fichier << " & " << Master_ptr->nbIntUpSet ;
        }
        else {
            fichier << " & - "  ;
        }
        fichier << " &  " << Master_ptr->nbIter ;
        fichier << " & " << SCIPgetNPricevarsFound(scip) ;

        fichier << " & " << Pricer->unitColumns ; // RL CPLEX
        fichier << " & " << Pricer->timeColumns ; // RL CPLEX
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
        fichier << " & " << checker.getLRValue() ; // RL*/
        fichier << " & " << checker.getLRCplex() ; // RL CPLEX
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
                // fichier << " & " << checker.getLRCplex() ; // RL CPLEX
                // if (0 && (met*intra_cons==102) || (!intra_cons*met==103)) {

                //     fichier << " & " << checker.getLRValue() ; // RL*/
                //     fichier << " & " << checker.getLRCplex() ; // RL CPLEX

                //     if (demande==4 || n < 20 || T < 48) {
                //         checker.getIntegerObjValue();
                //         fichier << " & " << checker.PrimalBound ; // OPT
                //     }
                //     else {
                //         fichier << " & {-} "  ; // OPT
                //     }

                // }
                // else{
                //     fichier << " & & & " ;
                // }
        fichier <<" \\\\ " << endl ;
       // checker.checkSolution(Master_ptr->x_frac);


        //ajout des couts réduits au fichier correspondant
        list<double>::const_iterator itv;
        for (itv = Master_ptr->totalDualCostList.begin(); itv!=Master_ptr->totalDualCostList.end(); itv++) {
            fichierDualCost << fixed << (*itv) << endl;
        }
        fichierDualCost << endl ;
        cout << scientific;
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
        fichier << " & " << checker.getLRCplex() ; // RL CPLEX
        fichier << " & " << checker.DualBound ;
        fichier << " & " << checker.PrimalBound ; // OPT
        fichier << " & - " ;
        fichier <<" \\\\ " << endl ;
    }



//    ////// AFFICHAGE PRICE AND BRANCH
//    if (param.ColumnGeneration) {
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

