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

/* namespace usage */
using namespace std;
using namespace scip;

//#define SCIP_OUTPUT

#define SCIP_DEBUG

int main(int argc, char** argv)
{
    if (argc==1) { // pour debug
        ofstream fichier("result.txt");


        //////////////////////////////
        //////  INSTANCE DATA    /////
        //////////////////////////////

        //Paramètres de l'instance
        int T = 5;
        int n = 3 ;
        int sym = 3 ;
        int demande = 3;
        int cat01 = 0;
        int bloc = 1;
        int intra = 1 ;

        string localisation = "data/" ;
        InstanceProcessed Instance = InstanceProcessed(n, T, bloc, demande, sym, cat01, intra, 1, localisation) ;

        string nom = Instance.fileName() ;
        const char* file = nom.c_str() ;

        IloEnv env;
        InstanceUCP* inst = new InstanceUCP(env, file) ;


        ////////////////////////////////////
        //////  SCIP INITIALIZATION    /////
        ////////////////////////////////////

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

        //SCIPsetLongintParam(scip, "limits/nodes", 5);

        /* set verbosity parameter */
        SCIPsetIntParam(scip, "display/verblevel", 5);
        //SCIPsetBoolParam(scip, "display/lpinfo", TRUE);

        /* create empty problem */
        SCIPcreateProb(scip, "UCP", 0, 0, 0, 0, 0, 0, 0);



        //////////////////////////////////////////////
        //////  MASTER PROBLEM INITIALIZATION    /////
        //////////////////////////////////////////////

        Master_Model Master(inst) ;
        Master.InitScipMasterModel(scip, inst) ;


        ////////////////////////////////
        //////  PRICING PROBLEM    /////
        ////////////////////////////////

        static const char* PRICER_NAME = "Pricer_UCP";

        // include UCP pricer
        ObjPricerUCP* pricer_ptr = new ObjPricerUCP(scip, PRICER_NAME, &Master, inst);

        SCIPincludeObjPricer(scip, pricer_ptr, true);

        // activate pricer
        SCIPactivatePricer(scip, SCIPfindPricer(scip, PRICER_NAME));


        cout<<"Write init pl"<<endl;
        SCIPwriteOrigProblem(scip, "init.lp", "lp", FALSE);



        //////////////////////////
        //////  BRANCHING    /////
        //////////////////////////

        BranchConsHandler* branchConsHandler = new BranchConsHandler(scip, pricer_ptr);
        BranchingRule* branchRule = new BranchingRule(scip, inst,  &Master, pricer_ptr);

        SCIPincludeObjConshdlr(scip, branchConsHandler, TRUE);
        SCIPincludeObjBranchrule(scip, branchRule, TRUE);


        //////////////////////
        //////  SOLVE    /////
        //////////////////////

        SCIPsolve(scip);


        //////////////////////
        //////  CHECK    /////
        //////////////////////

        CplexChecker checker = CplexChecker(inst) ;
        int value = checker.check() ;
        cout << "VALEUR OPTIMALE A TROUVER: " << value << endl ;

    }


    return 0;
}

