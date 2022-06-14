#ifndef INSTANCEUCP
#define INSTANCEUCP

#include <ilcplex/ilocplex.h>
#include <fstream>

class InstanceUCP {

private:
    IloEnv env ;

    ///// Recupéré du fichier instance /////

    IloInt n, T, K, S ;
    IloBoolArray Init ;
    IloIntArray L, l, nk, firstOfSite, SiteOf ;
    IloNumArray D, P, Pmax, cf, c0, cp ;

    IloIntArray First ; // i est le premier element d'un groupe d'unités identiques, First[i]=1
    IloIntArray Last ;

    ///// Calculé pendant l'initialisation /////
    IloInt SommePmax ;

    IloIntArray Tri ; // vecteur où les unités sont triées par Pmax décroissants

    //// sites
    IloIntArray firstUnitofSite ; // premières unités de chaque site

    //// indicateurs symétrie
    int nbGroupes ; //sans compter les unités seules
    int MaxSize ; //taille du plus gros groupe de symétrie
    double MeanSize ; //taille moyenne d'un groupe

    IloIntArray FirstG ; // tableau des premiers éléments de chaque groupe
    IloIntArray LastG ;
    IloIntArray SizeG ; // tailles de chaque groupe
    IloIntArray Group ; //group[i] = numéro du groupe de symétrie de l'unité i
    int nbG ; //nb de groupes en comptant les unités seules
    int nbG2 ; //nb de groupes de taille >= 2

    IloIntArray repartition_tailles ;

    //// indicateurs pour le branchement
    IloIntArray ordreT ;
    //t triés dans l'ordre de la demande décroissante

    //pour le branchement maison
    int tmax1 ;
    int tmax2 ;
    int tmin ;
    int Dmax ;
    int Dmin ;

    IloIntArray ordre_ratio ;
    int nbase ; // unités de base dans l'ordre ratio
    int np ; // unités de pic dans l'ordre ratio

    IloIntArray isIncreasing ;


public:

    InstanceUCP(IloEnv envir, const char* file) ;
    ~InstanceUCP() {

        Init.end() ;

        L.end() ;
        l.end() ;
        nk.end() ;
        firstOfSite.end() ;

        D.end() ;
        P.end() ;
        Pmax.end() ;
        cf.end() ;
        c0.end() ;
        cp.end() ;

        First.end() ; // i est le premier element de son groupe, First[i]=1
        Last.end() ;

        FirstG.end() ; // tableau des premiers éléments de chaque groupe
        LastG.end() ;
        Group.end() ; //group[i] = numéro du groupe de symétrie de l'unité i

        firstUnitofSite.end() ;

        //t triés dans l'ordre de la demande décroissante
        ordreT.end() ;

        ordre_ratio.end() ;
        isIncreasing.end() ;

        Tri.end() ;

    }

public:
    void Lecture(const char* file);
    void Initialise() ;

    IloEnv getenv() ;
    IloInt getn() const ;
    IloInt getT() const ;
    IloInt getS() const ;
    IloInt firstUnit(IloInt s) const ;
    IloInt getSiteOf(IloInt i) const;
    IloInt nbUnits(IloInt s) const ; // nombre d'unités sur le site s

    IloBool getInit(IloInt i) const ;
    IloInt getL(IloInt i) const ;
    IloInt getl(IloInt i) const ;
    IloNum getD(IloInt i) const ;
    IloNum getPmin(IloInt i) const ;
    IloNum getPmax(IloInt i) const ;
    IloNum getcf(IloInt i)  const ;
    IloNum getc0(IloInt i) const ;
    IloNum getcp(IloInt i) const ;

    IloNum getnbG() const;
    IloNum getFirst(IloInt i) const;
    IloNum getLast(IloInt i) const;
    IloNum getFirstG(IloInt i) const;
    IloNum getLastG(IloInt i) const;
    IloNum getSizeG(IloInt g) const;
    IloNum getGroup(IloInt i) const;
    IloNum getordreT(IloInt t) const ;

    IloInt getTri(IloInt i) const ;

    IloInt getSommePmax()  const ;

    IloInt partition(IloNumArray const & ordre, IloIntArray & indices, IloInt p, IloInt q) ;

    void quickSort(IloNumArray const & ordre, IloIntArray & indices, IloInt p, IloInt q) ;
    
    int id ;

};

#endif /* INSTANCEUCP_INCLUDED */
