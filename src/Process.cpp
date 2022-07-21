#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <math.h>
#include <ctime>
#include <list>

#include "InstanceUCP.h"
#include "Process.h"

using namespace std ;

string to_string2(int number){
    string number_string = "";
    char ones_char = '0';
    int ones = 0;
    while(true){
        ones = number % 10;
        switch(ones){
        case 0: ones_char = '0'; break;
        case 1: ones_char = '1'; break;
        case 2: ones_char = '2'; break;
        case 3: ones_char = '3'; break;
        case 4: ones_char = '4'; break;
        case 5: ones_char = '5'; break;
        case 6: ones_char = '6'; break;
        case 7: ones_char = '7'; break;
        case 8: ones_char = '8'; break;
        case 9: ones_char = '9'; break;
        default : ; //ErrorHandling("Trouble converting number to string.");
        }
        number -= ones;
        number_string = ones_char + number_string;
        if(number == 0){
            break;
        }
        number = number/10;
    }
    return number_string;
}



string InstanceProcessed::createName() const {
    string s_n = to_string2(n) + "_" ;

    string s_T = s_n + to_string2(T) ;
    s_T = s_T + "_" ;

    string s_bloc = s_T + to_string2(bloc);
    s_bloc = s_bloc + "_" ;

    string s_demande = s_bloc + to_string2(demande) ;
    s_demande = s_demande + "_" ;

    string  s_symetrie = s_demande + to_string2(symetrie) ;
    s_symetrie = s_symetrie + "_" ;

    string s_cat = s_symetrie + to_string2(cat) ;
    s_cat = s_cat + "_" ;

    string s_intra = s_cat + to_string2(intra) ;
    s_intra = s_intra + "_" ;

    string s_id = s_intra + to_string2(id) ;
    string file = s_id + ".txt" ;

    return s_id ;
}

string InstanceProcessed::fileName() {

        string nom = createName() ;
        string fileI = localisation + nom;
        string fileS = fileI + ".txt" ;
        cout << fileS << endl ;
        return fileS ;
}

Parameters::Parameters(InstanceUCP* inst, bool ColumnGeneration, int nodeLimit, bool ip, bool managesubpbsym, bool ramp,
                       bool time, bool intra, bool dr, bool iup, double eps,
                       bool dont, bool h_init, bool dontgetpvalue, bool one,
                       bool addColumn, bool dptime, bool dp, bool pandb,
                       bool unitdecomp, bool startupdec, bool useSSBISub,
                       bool PowerplanGivenByLambda, bool PowerplanGivenByMu, bool PminOnLambda, bool PmaxOnLambda, bool balanceCosts, int guidageRepartition, bool heurPricingTime, double heurPricingThreshold, bool PminDifferentPmax, bool rampmaster, bool rampsub,
                       bool ssbi, bool doubledec, bool rsu, bool minupdowndouble,
                       bool unitgeqtime, bool useuvar, bool dpsusd, bool nlsucost, bool Farkas, bool stopFirstSite, bool stopFirstTime, bool oneRoundTime) :
    ColumnGeneration(ColumnGeneration),
    nodeLimit(nodeLimit),
    IP(ip),
    ManageSubPbSym(managesubpbsym),
    Ramp(ramp),
    TimeStepDec(time),
    IntraSite(intra),
    DemandeResiduelle(dr),
    IntervalUpSet(iup),
    Epsilon(eps),
    DontPriceAllTimeSteps(dont),
    heuristicInit(h_init),
    DontGetPValue(dontgetpvalue),
    OneTimeStepPerIter(one),
    AddColumnToOtherTimeSteps(addColumn),
    DynProgTime(dptime),
    DynProg(dp),
    PriceAndBranch(pandb),
    UnitDecompo(unitdecomp),
    StartUpDecompo(startupdec),
    useSSBIinSubPb(useSSBISub),
    powerPlanGivenByLambda(PowerplanGivenByLambda),
    powerPlanGivenByMu(PowerplanGivenByMu),
    PminOnLambda(PminOnLambda),
    PmaxOnLambda(PmaxOnLambda),
    balanceCosts(balanceCosts),
    guidageRepartition(guidageRepartition),
    heurPricingTime(heurPricingTime),
    heurPricingThreshold(heurPricingThreshold),
    PminDifferentPmax(PminDifferentPmax),
    rampInMaster(rampmaster),
    rampInSubPb(rampsub),
    masterSSBI(ssbi),
    doubleDecompo(doubledec),
    RSUonly(rsu),
    minUpDownDouble(minupdowndouble),
    unitGEQTime(unitgeqtime),
    useUVar(useuvar),
    DynProgSUSD(dpsusd),
    nonLinearStartUpCost(nlsucost),
    Farkas(Farkas),
    stopFirstSite(stopFirstSite),
    stopFirstTime(stopFirstTime),
    oneRoundTime(oneRoundTime)
{


    n = inst->getn();
    siteOf = IloIntArray(inst->getenv(), n) ;


    for (int i=0 ; i < n ; i++) {
        if (UnitDecompo) {
            siteOf[i] = i ;
        }
        else {
            siteOf[i] = inst->getSiteOf(i);
        }
    }

    if (UnitDecompo) {
        nbDecGpes = inst->getn();
    }
    else {
        nbDecGpes = inst->getS() ;
    }

    nbUnitsGpe = IloIntArray(inst->getenv(), nbDecGpes) ;
    for (int s = 0 ; s < nbDecGpes ; s++) {
        if (UnitDecompo) {
            nbUnitsGpe[s] = 1 ;
        }
        else {
            nbUnitsGpe[s] = inst->nbUnits(s);
        }
    }

    firstUnitGpe = IloIntArray(inst->getenv(), nbDecGpes) ;
    for (int s = 0 ; s < nbDecGpes ; s++) {
        if (UnitDecompo) {
            firstUnitGpe[s] = s ;
        }
        else {
            firstUnitGpe[s] = inst->firstUnit(s);
        }
    }

    costBalancingMaster.resize(n,1);
    costBalancingPricer.resize(n,1);

    if (doubleDecompo){
        if (unitGEQTime){
            for (int i=0 ; i < n ; i++) {
                if (PmaxOnLambda){
                    costBalancingMaster.at(i) = (inst->getcf(i) + inst->getcp(i) * inst->getPmin(i)) / (inst->getcf(i) + inst->getcp(i) * inst->getPmax(i)) ;
                    if (balanceCosts){
                        costBalancingPricer.at(i) = (inst->getcf(i) + inst->getcp(i) * inst->getPmin(i)) / ( 2 * (inst->getcf(i) + inst->getcp(i) * inst->getPmax(i)) );
                    }
                    
                }
                else if (PminOnLambda){
                    //costBalancingMaster stays at 1
                    if (balanceCosts){
                        costBalancingPricer.at(i) = 0.5;
                    }

                }
                else {
                    costBalancingMaster.at(i) = 1 + inst->getcp(i) * inst->getPmin(i) / inst->getcf(i) ;
                    if (balanceCosts){
                        costBalancingPricer.at(i) = (inst->getcf(i) + inst->getcp(i) * inst->getPmin(i)) / ( 2 * inst->getcf(i) );
                    }
                }
            }
        }
        else {
            if (balanceCosts){
                for (int i=0 ; i < n ; i++) {
                    if (PmaxOnLambda){
                        costBalancingMaster.at(i) = (inst->getcf(i) + inst->getcp(i) * inst->getPmin(i)) / ( 2 * (inst->getcf(i) + inst->getcp(i) * inst->getPmax(i)) );
                        costBalancingPricer.at(i) = (inst->getcf(i) + inst->getcp(i) * inst->getPmin(i)) / ( 2 * (inst->getcf(i) + inst->getcp(i) * inst->getPmax(i)) );    
                    }
                    else if (PminOnLambda){
                        costBalancingMaster.at(i) = 0.5;
                        costBalancingPricer.at(i) = 0.5;
                    }
                    else {
                        costBalancingMaster.at(i) = (inst->getcf(i) + inst->getcp(i) * inst->getPmin(i)) / ( 2 * inst->getcf(i) );
                        costBalancingPricer.at(i) = (inst->getcf(i) + inst->getcp(i) * inst->getPmin(i)) / ( 2 * inst->getcf(i) );
                    }
                }
            }
        }
    }

}


int Parameters::nbUnits(int s) const  {
    if (s >= 0 && s < nbDecGpes) {
        return nbUnitsGpe[s] ;
    }
    else {
        cout << "indice site incorrect: " << s << endl;
        return -1 ;
    }
}

int Parameters::firstUnit(int s) const {
    if (s >= 0 && s < nbDecGpes) {
        return firstUnitGpe[s] ;
    }
    else {
        cout << "indice site incorrect: " << s << endl;
        return -1 ;
    }
}

int Parameters::getSiteOf(int i) const {
    if (i >= 0 && i < n) {
        return siteOf[i] ;
    }
    else {
        cout << "indice unit incorrect: " << i << endl;
        return -1 ;
    }
}


Parameters init_parameters(InstanceUCP* inst, int met, int intra_cons) {

    bool ColumnGeneration = true ; // true: Branch & Price avec SCIP. false: résolution Cplex boîte noire

    if (met==-1) {
        ColumnGeneration= false ;
    }

    double eps = 0.0000001; // tolérance
    int node_limit =1000000000;

    bool IP=1; // est-ce qu'on résout le master en variable entières ?
    bool PriceAndBranch = 0;

    bool IntraSite = intra_cons ; // contraintes intrasites prises en compte ou pas

    bool nonLinearStartUpCost = false ; 
    //// Paramètres: type de décomposition ////
    bool UnitDecompo = false ;
    bool StartUpDecompo = false;


    bool TimeStepDec = 0 ;
    bool DynProgTime = 0 ; // implémenté pour Pmax=Pmin et décomposition par pas de temps
    bool powerPlanGivenByMu = false ; // décomposition par pas de temps pour Pmax!=Pmin (avec les puissances dans le ss pb)
    bool PminDifferentPmax = false; // décomposition par pas de temps pour Pmax!=Pmin (avec les puissances dans le pb maitre)

    bool DynProg = 0; // implémenté pour Pmax=Pmin et décomposition par unités
    bool DynProgSUSD =0 ; //algo de programmation dynamique avec uniquement des arcs d'arrêt/démarrage. A utiliser avec l'option DynProg=1

    bool DemandeResiduelle = 0 ;

// double décompo
    bool doubleDecompo = 0 ;
    bool minUpDownDouble = 0 ; // inégalités min-up/min-down dans le maître de la double décomposition, pour stabilisation
    bool UnitGEQTime = 0 ; // contrainte d'inégalité entre point(time) et point(unit), dans le maître de la double décomposition
    bool useUVar = 0 ;
    bool PminOnLambda = false;
    bool PmaxOnLambda = false;
    /////////////////////////////////////////

    bool balanceCosts = false ;
    int guidageRepartition = 0 ;

    bool heurPricingTime = false;
    double heurPricingThreshold = 0;

    //// Paramètres symétries et interval up-set ///
    bool IntervalUpSet = 0 ;// implémenté pour time decomposition seulement (résolution dyn prog)
    bool masterSSBI = 0 ; // implémenté pour time decomposition seulement (résolution dyn prog)
    bool RSUonly=0 ;

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
    bool stopFirstSite = false ;
    bool stopFirstTime = false ;
    bool oneRoundTime = false ;

    bool Farkas = false ;



    // Parse met value given as argument to infer parameters

    int arr[16];
    int indice = 0;
    int chiffre ;

    // On récupère tous les chiffres de met
    while(met != 0) {
        chiffre = met % 10 ;
        arr[indice] = chiffre ;
        indice++ ;
        met = met / 10 ;
    }

    // Puis on regarde les chiffres de gauche à droite pour fixer les paramètres

    // Type de décomposition
    switch (arr[indice - 1]) {
        case 1:
            // Par unité
            UnitDecompo = true ;
            DynProg = 1 ;
            break ;

        case 2:
            // Par pas de temps 
            TimeStepDec = true ;
            DynProgTime = true ;
            break ;

        case 3:
            // Double, Pmin = Pmax
            doubleDecompo = true ;
            UnitDecompo = true ;
            PminOnLambda = true ;
            DynProg = true ;
            DynProgTime = true ;
            break ;

        case 4:
            // Double, variables p dans le ss-pb par pas de temps
            doubleDecompo = true ;
            UnitDecompo = true ;
            PminDifferentPmax = true ;
            powerPlanGivenByMu = true ;
            DynProg = true ;
            break ;

        case 5:
            // Double, variables p dans le pb maitre
            doubleDecompo = true ;
            UnitDecompo = true ;
            PminDifferentPmax = true ;
            PminOnLambda = true ;
            DynProg = true ;
            DynProgTime = true ;
            break ;
    }

    // Branchement ou non
    if (arr[indice - 2] == 0) {
        node_limit = 1 ;
    }

    // Initialisation heuristique ou non
    if (arr[indice - 3] == 1) {
        heuristicInit = true ;
    }

    // Réglages spécifiques double decomposition
    if (doubleDecompo){
        switch (arr[indice - 4]) {
            // Cas 0 : basique, egalite et sans repartition de couts

            case 1:
                balanceCosts = true ;
                break ;

            case 2:
                UnitGEQTime = true ;
                break ;

            case 3:
                // Guidage 'naif' par la repartition
                UnitGEQTime = true ;
                balanceCosts = true ;
                break ;

            case 4:
                // Decroissance linéaire de la repartition
                UnitGEQTime = true ;
                balanceCosts = true ;
                guidageRepartition = 1 ;
                break ;

            case 5:
                // Decroissance linéaire de la repartition : version 2
                UnitGEQTime = true ;
                balanceCosts = true ;
                guidageRepartition = 2 ;
                break ;

            case 6:
                // Decroissance exp de la repartition
                UnitGEQTime = true ;
                balanceCosts = true ;
                guidageRepartition = 3 ;
                break ;
        }
    }
    else if (TimeStepDec){
        if (indice >= 4){
            DynProgTime = false ;
            if (arr[indice - 4] == 1){
                PminDifferentPmax = true ;
                powerPlanGivenByMu = true ;
            }       
        }
    }
    else{
        if (indice >= 4){
            DynProg = false ;
        }
    }

    if (indice >= 5){
        DynProg= false ;
    }

    if (indice >= 6){
        DynProgTime = false ;
    }


    Parameters param(inst, ColumnGeneration, node_limit, IP, ManageSubPbSym, Ramp, TimeStepDec, IntraSite, DemandeResiduelle, IntervalUpSet, eps, DontPriceAllTimeSteps,
                           heuristicInit, DontGetPValue, OneTimeStepPerIter, addColumnToOtherTimeSteps, DynProgTime, DynProg, PriceAndBranch,
                           UnitDecompo, StartUpDecompo, useSSBIinSubPb, powerPlanGivenByLambda, powerPlanGivenByMu, PminOnLambda, PmaxOnLambda, balanceCosts, guidageRepartition, heurPricingTime, heurPricingThreshold, PminDifferentPmax, RampInMaster, RampInSubPb, masterSSBI, doubleDecompo,RSUonly,
                           minUpDownDouble, UnitGEQTime, useUVar, DynProgSUSD, nonLinearStartUpCost, Farkas, stopFirstSite, stopFirstTime, oneRoundTime);


    return param;

}