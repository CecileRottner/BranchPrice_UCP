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

using namespace std ;

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

    string fileName() {

        string nom = createName() ;
        string fileI = localisation + nom;
        string fileS = fileI + ".txt" ;
        cout << fileS << endl ;
        return fileS ;
    }

};

#endif 
