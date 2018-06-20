#ifndef SEPARATION
#define SEPARATION

#include <ilcplex/ilocplex.h>
#include <string>
#include <list>
#include <vector>

#include "InstanceUCP.h"

using namespace std;

class Separation {

public:
    IloEnv env ;


public : // à remettre protected après les tests
    InstanceUCP* pb ;

    IloNum eps ;

    IloInt n ;
    IloInt T ;

    IloInt Lmax ;
    IloInt lmax ;

    double sommeP ;

    int** indiceDmax ;
    //indiceDmax[t0][t] : index de la demande max sur l'intervalle [t0, t0 + t + 1]

    int*** Dl ;
    //Dl[t0][t][l] : indice du min des Dyj pour intervalle [t0, t0+t+1] pour \ell = l+1

    int** alphaMax ;
    //bornes sup stupides sur alpha
    //alphaMax[t0][t] = borne sup sur alpha pour l'intervalle [t0, t0+t+1]

    int* alphaMax_t ; // alpha maximum possible au pas de temps t, qq soit C

    int** existe_i;

    //Dépend de l'intervalle [t0, t1] et de la solution fractionnaire (xx,uu) courants. Mis à jour par computeCosts, computeWeightedCost et computeSommeU
    IloNumArray coutW ;
    IloNumArray coutZ ;
    IloNumArray WeightedCost ;
    IloIntArray indices ;
    IloIntArray indicesPureCost ;
    IloIntArray savedIndices ;
    IloNumArray SommeU ;

public :
    //////////// Méthodes de séparation ///////////////////
    IloInt separeGlouton(IloInt t0, IloInt t1, IloInt i, IloIntArray & C) ;
    //Préconditions : couts à jour (coutW, coutZ, WeightedCost, SommeU)
    //affecte à C un C qui viole l'inégalité
    //valeur de retour : alpha de C si C violé, 0 sinon

    IloInt separeRandom(IloInt t0, IloInt t1, IloInt i, IloIntArray & C) ;

    IloInt separeCover(IloInt t, IloIntArray & C) ;
    //Préconditions: couts à jour, vecteur indices trié
    //affecte à C un C qui viole l'inégalité de Cover associée
    //valeur de retour : alpha de C si C viole, 0 sinon

    IloInt separeLetchford(IloInt t, IloIntArray & C) ;
    //heuristique de Letchford

    IloInt separeExact(IloInt t0, IloInt t1, IloInt & i, IloIntArray & C) ;
    //Préconditions : couts à jour (coutW, coutZ)
    //retournent un (C, i) qui viole l'inégalité pour t0, t1
    //Si elle ne trouve pas un tel C, alors C est un vecteur vide (de taille 0)
    //retourne le alpha pour lequel l'inégalité est violée. Si alpha=0 alors on n'a pas trouvé d'inégalité violée.


    ///////////    Fonctions annexes   ////////////////////

    void computeIndDmax(int t0, int t1) ;

    IloInt inserePuissance(IloNum D) ;
    //retourne k tel que P(k+1)max < D <= Pkmax
    // 0 si D > P1max

    IloInt getIndDmax(IloInt t0, IloInt t1) ;

    IloInt getDmax(IloInt t0, IloInt t1) ;
    //retourne la demande max sur l'intervalle [t0, t1]

    IloInt getDl(IloInt t0, IloInt t1, IloInt l) ;
    //retourne Dl (min sur les yj) sur l'intervalle [t0, t1], pour les yj associé à l (dans [1, lmax])

    IloInt getAlphaMax(IloInt t0, IloInt t1) ;
    //retourne la alpha max sur l'intervalle [t0, t1] - pas prévu pour un intervalle de type [t0, t0]

    IloInt getExiste_i(IloInt t0, IloInt t1, IloInt l) ;

    IloInt isUpSet(IloInt t, const IloIntArray & C, IloInt sommeC) ;

    IloInt ComputeAlpha(IloInt t, const IloIntArray & C, IloInt sommeC) ;
    //Calcul de alpha(C,t), C étant trié par Pmax décroissants

    void updateAlpha(int & alpha, IloIntArray & C, IloInt c, double & Dtilde, double & lastUsed, double & sommePUsed, IloInt j) ;

    IloInt ComputeAlpha_i(IloInt t, IloInt i, const IloIntArray & C, IloInt sommeC) ;
    //Calcul de alpha(C,t)^i, C étant trié par Pmax décroissants
    //Si i ne peut pas être éteint (condition de pleine dimension non vérifiée) alors renvoie n

    void ComputeBetas_i(IloIntArray & Betas, IloInt t0, IloInt t1, IloInt i, const IloIntArray & C, IloInt sommeC, IloInt alphaMax) ;

    IloInt alphaMax_C(const IloIntArray & C, IloInt t0, IloInt t1, IloInt sommeC) ;

    IloBool AlphaOK(const IloIntArray & C, IloInt i, IloInt t0, IloInt t1, IloInt sommeC, IloInt alpha_max) ;
    //vérifie les conditions C1 et C2

    IloBool iOK(IloInt i, IloInt t0, IloInt t1) ;
    // 1 ssi Li >= t1 - t0

    IloBool iStarts(IloInt i) ;
    //1 si i démarre sur intervalle en cours

    IloInt ComputeBound(IloInt t0, IloInt t1) ;
    //Calcule la borne sup générale (ne dépendant que de D) sur alpha pour l'intervalle [t0, t1]

    void computeCosts(IloInt t0, IloInt t1, const vector<double> & xx, const vector<double> & uu) ;
    //Calcul de coutY et coutZ pour intervalle [t0, t1] et (xx,uu) donnés

    void computeWeightedCosts(IloInt t0, IloInt t1, const vector<double> & xx, const vector<double> & uu) ;
    //calcule les contributions pondérées WeightedCost[j] = (1/Pjmax)*(x(j, t0) + Sum(t0+1, t1) u(j, t)) de chaque unité, et trie le vecteur indices par WeightedCost croissants

    int getS(int alpha, const IloIntArray & C, IloIntArray & S) ;
    int getMinimalCover(IloIntArray & C, IloInt t, IloInt sommeC) ;
    void getStrong(IloIntArray & C, IloInt t, IloInt sommeC)  ;
    int getExt(IloIntArray & C) ; //returns alpha(E(S))
    int getExt_(const IloIntArray & C, int c_size, IloIntArray & S, int & valS) ; //place l'extension dans S, et retourne alpha(S), met à jour valS

    int getFacet(int alpha, int t, const IloIntArray & C, IloIntArray & S) ;//returns alpha(C)
    int no1contrib(IloIntArray & C) ;
    bool isFacet(IloIntArray & S, IloInt t) ;
    //renvoie 1 si l'inégalité associée à E(S) est facette, avec S strong

    int getMinimal(IloIntArray & C, IloInt t0, IloInt t1, IloInt i, IloInt SommeC) ;
    //Enlève des éléments de C tant que ça permet de conserver la violation de l'inégalité. retourne le nouvel alpha correspondant

    int insert(IloIntArray & C, IloInt i) ;
    // insère i dans C trié de manière à ce que C reste trié (par P décroissants), renvoie l'indice de i dans C



    void remove(IloIntArray & result, const IloIntArray & C, IloInt c, IloInt j) ;
    void removeEnPlace(IloIntArray & C, IloInt c, IloInt j);
    //c : dimension de C



    IloInt yDefine(IloIntArray & Y, int t0, int t1, int l) ;
    //renvoie la suite (yj) dans le vecteur Y

    IloBool ICviolee(const IloIntArray & C, IloInt i, IloInt alpha, double valeur) ;
    //Préconditions: coutW et coutZ à jour pour intervalle [t0, t1] et solution (xx,uu)

    IloBool Cviolee(const IloIntArray & C, IloInt alpha, double valeur) ;

    void saveIndices() ;
    void copyIndices() ;


public:

    Separation(InstanceUCP* inst) ;
    ~Separation() ;

    IloInt getn() ;
    IloInt getT() ;
    IloInt getL(IloInt i) ;
    IloInt getl(IloInt i) ;
    IloInt getPmax(IloInt i) ;

    int getLmax() ;



    void permut(IloIntArray & C) ;
    //fait une permutation aléatoire des éléments de C


    void quickSort(IloNumArray const & ordre, IloIntArray & indices, IloInt p, IloInt q) ;

    IloInt partition(IloNumArray const & ordre, IloIntArray & indices, IloInt p, IloInt q) ;

    int SepareSCIP(list<int>* C_list, IloInt i, IloInt t0, IloInt t1) ;
};


#endif /* SEPARATION_INCLUDED */
