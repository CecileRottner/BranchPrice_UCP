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

Parameters::Parameters(InstanceUCP* inst, bool ip, bool managesubpbsym, bool ramp,
                       bool time, bool intra, bool dr, bool iup, double eps,
                       bool dont, bool h_init, bool dontgetpvalue, bool one,
                       bool addColumn, bool dptime, bool dp, bool pandb,
                       bool unitdecomp, bool startupdec, bool useSSBISub,
                       bool powerplanGiven, bool rampmaster, bool rampsub,
                       bool ssbi, bool doubledec) :
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
    powerPlanGivenByLambda(powerplanGiven),
    rampInMaster(rampmaster),
    rampInSubPb(rampsub),
    masterSSBI(ssbi),
    doubleDecompo(doubledec)
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
