#ifndef PROCESSS
#define PROCESSS

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
using namespace std ;

class Parameters
{
public:
    bool ColumnGeneration;
    int nodeLimit;

    bool IP ;
    bool ManageSubPbSym;
    bool Ramp ;
    bool TimeStepDec ;
    bool IntraSite ;
    bool DemandeResiduelle ;
    bool IntervalUpSet ;
    double Epsilon ;
    bool DontPriceAllTimeSteps ;
    bool heuristicInit ;
    bool DontGetPValue ;
    bool OneTimeStepPerIter ;
    bool AddColumnToOtherTimeSteps ;
    bool DynProgTime ;
    bool DynProg ;
    bool PriceAndBranch ;
    bool UnitDecompo ;
    bool StartUpDecompo ;
    bool useSSBIinSubPb ;
    bool powerPlanGivenByLambda;
    bool powerPlanGivenByMu;
    bool PminOnLambda;
    double costBalancing;
    bool PminDifferentPmax;
    bool rampInMaster;
    bool rampInSubPb ;
    bool masterSSBI;
    bool doubleDecompo ;
    bool RSUonly ;
    bool minUpDownDouble ;
    bool unitGEQTime ;
    bool useUVar ;
    bool DynProgSUSD;
    bool nonLinearStartUpCost ;


    /// Sites (ou groupes) définis par Param: groupes d'unités de la décomposition (correspondant à des sousproblème)
    /// Sites définis dans inst: sites sur lesquels vaut la contrainte d'intrasite

    int nbDecGpes ;
    int n;
    IloIntArray nbUnitsGpe ;
    IloIntArray firstUnitGpe ;
    IloIntArray siteOf ;

    Parameters(InstanceUCP* inst, bool ColumnGeneration, int nodeLimit, bool ip, bool managesubpbsym, bool ramp,
               bool time, bool intra, bool dr, bool iup, double eps,
               bool dont, bool h_init, bool dontgetpvalue, bool one,
               bool addColumn, bool dptime, bool dp, bool pandb,
               bool unitdecomp, bool startupdec, bool useSSBISub,
               bool PowerPlanGivenByLambda, bool PowerPlanGivenByMu, bool PminOnLambda, double CostBalancing, bool PminDifferentPmax, bool rampMaster, bool rampSubpb,
               bool masterSSBI, bool doubleDecompo, bool RSUonly, bool minUpDownDouble,
               bool unitgeqtime, bool useuvar, bool dpsusd, bool nlsucost) ;

    int nbUnits(int s) const ;

    int firstUnit(int s) const ;

    int getSiteOf(int i) const;

    int test() {
        cout << "size: " << siteOf.getSize() << endl ;
        return siteOf[0] ;
    }
};

class InstanceProcessed {
public :

    //donnees
    int n ;
    int T ;
    int bloc ;
    int demande ;
    int symetrie ;
    int cat ;
    int intra ;
    int id ;

    //Methode
    string localisation ;

    InstanceProcessed(int n_, int T_, int bloc_, int demande_, int symetrie_, int cat_, int intra_, int id_, string loc_)
    {
        n=n_ ;
        T=T_ ;
        bloc = bloc_ ;
        demande = demande_ ;
        symetrie = symetrie_ ;
        cat = cat_ ;
        intra = intra_ ;
        id = id_ ;
        localisation = loc_ ;
    }

    string createName() const ;

    string fileName() ;

};



// initializes Parameters class using met indicator
Parameters init_parameters(InstanceUCP* inst, int met, int intra_const) ;

#endif 
