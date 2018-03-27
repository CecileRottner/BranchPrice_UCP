#ifndef INSTANCEUCP
#define INSTANCEUCP

#include <ilcplex/ilocplex.h>
#include <fstream>

class InstanceUCP {

private:
    IloEnv env ;
    int id ;

    IloInt n, T, K, S ;
    IloBoolArray Init ;
    IloIntArray L, l, nk, firstOfSite ;
    IloNumArray D, P, Pmax, cf, c0, cp ;

    IloIntArray First ; // i est le premier element de son groupe, First[i]=1
    IloIntArray Last ;

    IloIntArray First_ ; // i est le premier element de son groupe, First[i]=1
    IloIntArray Last_ ;

    IloIntArray FirstG_ ; // tableau des premiers éléments de chaque groupe
    IloIntArray LastG_ ;
    IloBoolArray Init_ ;
    IloIntArray L_, l_, nk_ ;
    IloNumArray P_, Pmax_, cf_, c0_ , cp_;

    IloIntArray C_ ;

    IloInt SommePmax ;

    IloIntArray firstUnitofSite ;

    //indicateurs symétrie
    int nbGroupes ; //sans compter les unités seules
    int MaxSize ; //taille du plus gros groupe de symétrie
    double MeanSize ; //taille moyenne d'un groupe

    IloIntArray FirstG ; // tableau des premiers éléments de chaque groupe
    IloIntArray LastG ;
    IloIntArray SizeG ;
    IloIntArray Group ; //group[i] = numéro du groupe de symétrie de l'unité i
    int nbG ; //nb de groupes en comptant les unités seules
    int nbG2 ; //nb de groupes de taille >= 2

    IloIntArray repartition_tailles ;

    //t triés dans l'ordre de la demande décroissante
    IloIntArray ordreT ;


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

        Init_.end() ;
        L_.end() ;
        l_.end() ;
        nk_.end() ;

        P_.end() ;
        Pmax_.end() ;
        cf_.end() ;
        c0_.end() ;
        cp_.end() ;

        C_.end() ;


        FirstG.end() ; // tableau des premiers éléments de chaque groupe
        LastG.end() ;
        Group.end() ; //group[i] = numéro du groupe de symétrie de l'unité i

        //t triés dans l'ordre de la demande décroissante
        ordreT.end() ;

        ordre_ratio.end() ;
        isIncreasing.end() ;
    }

public:
    void Lecture(const char* file);
    void Initialise() ;

    IloInt getn() const ;
    IloInt getT() const ;
    IloInt getS() const ;
    IloInt firstUnit(IloInt s) const ;
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

    IloInt getSommePmax()  const ;

    IloInt partition(IloNumArray const & ordre, IloIntArray & indices, IloInt p, IloInt q) ;

    void quickSort(IloNumArray const & ordre, IloIntArray & indices, IloInt p, IloInt q) ;

};

#endif /* INSTANCEUCP_INCLUDED */
