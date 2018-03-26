#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

/* scip includes */
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

/* user defined includes */
#include "InstanceUCP.h"
#include "Process.h"
#include "Master.h"

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
        int T = 24;
        int n = 10 ;
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
        InstanceUCP inst = InstanceUCP(env, file) ;


        ////////////////////////////////////
        //////  SCIP INITIALIZATION    /////
        ////////////////////////////////////

        // problem initialization
        SCIP *scip=NULL;
        SCIPcreate(&scip);

        SCIPprintVersion(scip, NULL);
        SCIPinfoMessage(scip, NULL, "\n");

        /* include default plugins */
        SCIPincludeDefaultPlugins(scip);

        /* set verbosity parameter */
        SCIPsetIntParam(scip, "display/verblevel", 5);
        /* SCIPsetBoolParam(scip, "display/lpinfo", TRUE); */

       /* create empty problem */
        SCIPcreateProb(scip, "UCP", 0, 0, 0, 0, 0, 0, 0);

        //////////////////////////////////////////////
        //////  MASTER PROBLEM INITIALIZATION    /////
        //////////////////////////////////////////////

        Master_Model Master(inst) ;
        Master.InitScipMasterModel(scip, inst) ;


    }

    return 0;
}

