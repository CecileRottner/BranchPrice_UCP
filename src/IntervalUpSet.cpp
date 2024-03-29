#include "IntervalUpSet.h"
#include "InstanceUCP.h"

#include <ilcplex/ilocplex.h>
#include <string>
#include <list>

using namespace std ;

Separation::Separation(InstanceUCP* inst, double epsilon) :
    pb (inst),
    eps(epsilon)
    {
    n = pb->getn();
    T = pb->getT();


    coutW = IloNumArray(env, n);
    coutZ = IloNumArray(env, n);
    WeightedCost = IloNumArray(env, n);
    indices = IloIntArray(env, n) ;
    indicesPureCost = IloIntArray(env, n) ;
    savedIndices = IloIntArray(env, n) ;

    SommeU = IloNumArray(env, n);

    for (IloInt j = 0 ; j <n ; j++ ) {
        indices[j] = j ;
        indicesPureCost[j] = j ;
    }

    IloIntArray C(env,n) ;
    IloInt SommeC = 0 ;
    for (IloInt j=0 ; j < n ; j++) {
        C[j] = j ;
        SommeC += getPmax(j) ;
    }

    sommeP = SommeC ;

    Lmax = getL(0);
    for (IloInt j=1 ; j < n ; j++) {
        if (Lmax < getL(j)) {
            Lmax = getL(j);
        }
    }

    lmax = getl(0);
    for (IloInt j=1 ; j < n ; j++) {
        if (lmax < getl(j)) {
            lmax = getl(j);
        }
    }

    //indiceDmax[t0][t] : indice de la demande maximum sur l'intervalle [t0, t0+t+1] 0<=t<Lmax
    indiceDmax= new int*[T-1] ;
    for (int t = 0 ; t < T-1 ; t++) {
        indiceDmax[t] = new int[Lmax] ;
    }

    for (int t0 = 0 ; t0 < T-1 ; t0++) {
        if (t0 + Lmax >= T) {
            computeIndDmax(t0, T-2-t0) ;
            for (int t1=T-1-t0 ; t1 < Lmax ; t1++) {;
                indiceDmax[t0][t1] = 0 ;
            }
        }
        else {
            computeIndDmax(t0, Lmax-1) ;
        }
    }


    //Pour chaque intervalle, indice du yj qui minimise la demande sur l'intervalle
    Dl = new int**[T-1] ;
    for (int t = 0 ; t < T-1 ; t++) {
        Dl[t] = new int*[Lmax] ;
        for (int L = 0 ; L < Lmax ; L++) {
            Dl[t][L] = new int[lmax] ;
        }
    }

    for (int t0 = 0 ; t0 < T-1 ; t0++) {
        for (int t1=0 ; t1 < Lmax ; t1 ++) {
            if (t0+1+t1 < T) {
                for (int l = 0 ; l < lmax ; l++) {
                    IloIntArray Y(env,0);
                    int sizeY = yDefine(Y, t0, t0+t1+1, l+1) ;
                    int ymin = 0 ;
                    for (int y = 1 ; y < sizeY ; y++) {
                        if (pb->getD(Y[y]) < pb->getD(Y[ymin])) {
                            ymin = y;
                        }
                    }
                    Dl[t0][t1][l] = Y[ymin];
                }
            }
        }
    }




    //Bornes sur alpha
    alphaMax = new int*[T-1] ;
    for (int t = 0 ; t < T-1 ; t++) {
        alphaMax[t] = new int[Lmax] ;
    }

    for (int t0 = 0 ; t0 < T-1 ; t0++) {
        for (int t1=0 ; t1 < Lmax ; t1 ++) {
            if (t0+1+t1 < T) {
                alphaMax[t0][t1] = ComputeAlpha(indiceDmax[t0][t1], C, SommeC) ;
            }
            else {
                alphaMax[t0][t1] = 0 ;
            }
        }
    }

    alphaMax_t = new int[T] ;

    for  (int t = 0 ; t < T-1 ; t++) {
        alphaMax_t[t] = ComputeAlpha(t, C, SommeC) ;
    }



    //existe_i[L][l] : 1 si il existe une unité i telle que Li >= L et li = l
    existe_i = new int*[Lmax] ;
    for (int t = 0 ; t < Lmax ; t++) {
        existe_i[t] = new int[lmax] ;
    }

    for (int L = 0 ; L < Lmax ; L++) {
        for (int t1=0 ; t1 < lmax ; t1 ++) {
            existe_i[L][t1] = 0 ;
        }
    }


    for (int i = 0 ; i < n ; i++) {
        for (int L = 0 ; L < getL(i) ; L++) {
            existe_i[L][getl(i)-1] = 1 ;
        }
    }

}


Separation::~Separation() {

    for (int t = 0 ; t < T-1 ; t++) {
        delete[] indiceDmax[t];
    }
    delete[] indiceDmax ;

    for (int t = 0 ; t < T-1 ; t++) {
        delete[] alphaMax[t];
    }
    delete[] alphaMax ;


    for (int t = 0 ; t < T-1 ; t++) {

        for (int L = 0 ; L < Lmax ; L++) {
            delete[] Dl[t][L] ;
        }

        delete[] Dl[t] ;
    }

    delete[] Dl ;


    for (int t = 0 ; t < Lmax-1 ; t++) {
        delete[] existe_i[t] ;
    }

    delete[] existe_i  ;
}


////////////////// Fonctions "get_x" ///////////////////

IloInt Separation::getn() {
    return n ;
}

IloInt Separation::getT() {
    return T ;
}

IloInt Separation::getL(IloInt i) {
    if (i >= pb->getn()) {
        cout << "i < n dans getL" << endl;
    }
    return pb->getL(pb->getTri(i)) ;
}

IloInt Separation::getl(IloInt i) {
    if (i >= pb->getn()) {
        cout << "i < n dans getl" << endl;
    }
    return pb->getl(pb->getTri(i)) ;
}

IloInt Separation::getExiste_i(IloInt t0, IloInt t1, IloInt l) {
    return existe_i[t1 - t0 - 1][l-1] ;
}

int Separation::getLmax() {
    return Lmax ;
}

IloInt Separation::getPmax(IloInt i) {
    if (i >= pb->getn()) {
        cout << "i < n dans getPmax" << endl;
    }
    return pb->getPmax(pb->getTri(i)) ;
}

IloInt Separation::getIndDmax(IloInt t0, IloInt t1) {
    if (t0==t1) {
        return t0 ;
    }
    else {
        return indiceDmax[t0][t1 - t0 - 1] ;
    }
}

IloInt Separation::getDmax(IloInt t0, IloInt t1) {
    if (t0==t1) {
        return pb->getD(t0) ;
    }
    else {
        return pb->getD(indiceDmax[t0][t1 - t0 - 1]) ;
    }
}

IloInt Separation::getDl(IloInt t0, IloInt t1, IloInt l) {

    if (t0==t1) {

        return t0 ;
    }
    else {
        return Dl[t0][t1 - t0 - 1][l-1] ;
    }
}

IloInt Separation::getAlphaMax(IloInt t0, IloInt t1) {
    if (t0==t1) {
        return n ;
    }
    else {
        return alphaMax[t0][t1 - t0 - 1] ;
    }
}

void Separation::saveIndices() {
    for (int i = 0 ; i < n ; i++) {
        savedIndices[i] = indices[i];
    }
}

void Separation::copyIndices() {
    for (int i = 0 ; i < n ; i++) {
        indices[i] = savedIndices[i];
    }
}

///////////////////////////////////////////////////////////


//Calcul l'indice à demande maximale dans l'intervalle [t0, t] pour tout t dans [t0+1, t0+t1+1] et stocke le résultat dans indiceDmax[t0][t-t0-1]
//Si t0+t1+1 > T: ne fais rien
void Separation::computeIndDmax(int t0, int t1) {
    if (t0 + t1 + 1 >= T)  {
    }
    else if (t1 == 0) {
        if (pb->getD(t0) < pb->getD(t0+1)) {
            indiceDmax[t0][t1] = t0+1 ;
        }
        else {
            indiceDmax[t0][t1] = t0 ;
        }

    }

    else {
        computeIndDmax(t0, t1-1) ;
        int indice = indiceDmax[t0][t1-1];
        int indiceMax ;
        if (pb->getD(indice) < pb->getD(t0 + t1 + 1)) {
            indiceMax = t0 + t1 + 1;
        }
        else {
            indiceMax = indice;
        }
        indiceDmax[t0][t1] = indiceMax ;
    }
}

/// Calcule la suite (yj) sur l'intervalle [t0, t1]
IloInt Separation::yDefine(IloIntArray & Y, int t0, int t1, int l) {
    int size = 1 ;
    Y.add(t0);
    IloInt y = t0 ;
    while (t1 - y > l) {
        y = getIndDmax(y+1, y+l) ;
        Y.add(y) ;
        size++ ;
    }
    if (y < t1) {
        Y.add(t1) ;
        size++ ;
    };
    return size;
}

////// Mise à jour des coûts et des valeurs utiles ////////

void Separation::computeCosts(IloInt t0, IloInt t1, const vector<double> & xx, const vector<double> & uu) {
    //Calcul de coutW et coutZ pour intervalle [t0, t1] et (xx,uu) donnés

    //coutW
    for (IloInt j = 0 ; j < n ; j++) {
        int unit = pb->getTri(j) ;
        coutW[j] = xx[T*unit + t0] ;
        for (IloInt k = t0+1 ; k <= t1 ; k++) {
            coutW[j] += uu[unit*T + k] ;
        }
    }

    //coutZ
    for (IloInt j = 0 ; j < n ; j++) {
        int unit = pb->getTri(j) ;
        coutZ[j] = xx[T*unit + t1] ;
        for (IloInt k = t0+1 ; k <= t1 ; k++) {
            coutZ[j] -= uu[unit*T + k] ;
        }
    }

    //Tri du vecteur indicesPureCost est fait dans les fonction getMinimal, donc seulement si on en a besoin
    if (t0==t1) {
        pb->quickSort(coutW,indicesPureCost, 0, n);
    }
}

void Separation::computeWeightedCosts(IloInt t0, IloInt t1, const vector<double> & xx, const vector<double> & uu) {

    //Calcul
    for (IloInt j = 0 ; j < n ; j++) {
        int unit = pb->getTri(j) ;
        WeightedCost[j] = xx[T*unit + t0] ;
        for (IloInt k = t0+1 ; k <= t1 ; k++) {
            WeightedCost[j] += uu[unit*T + k] ;
        }
        WeightedCost[j] = WeightedCost[j]/getPmax(j);
    }

    //Tri du vecteur indices

    quickSort(WeightedCost,indices, 0, n);


}

/////////////////////////////////////////////////////////////////


//////////////// Calculs utiles séparation //////////////////////

IloInt Separation::isUpSet(IloInt t, const IloIntArray & C, IloInt sommeC) {
    int sommeCbar = pb->getSommePmax() ;
    sommeCbar -= sommeC ;
    int Dadjust = pb->getD(t) - sommeCbar ;
    return (Dadjust > 0) ;
}

IloInt Separation::ComputeAlpha(IloInt t, const IloIntArray & C, IloInt sommeC) {
    // cout << "Start compute alpha" << endl ;
    IloInt alpha = 0 ;
    int sommeCbar = pb->getSommePmax() ;
    sommeCbar -= sommeC ;
    int Dadjust = pb->getD(t) - sommeCbar ;
    int j = 0 ;
    int c = C.getSize() ;
    while (Dadjust > 0) {
        if (j ==c) {
            cout << "erreur (ComputeAlpha) : plus d'éléments dans C pour calculer alpha" << endl ;
        }
        alpha ++ ;
        Dadjust -= getPmax(C[j]) ;
        j++ ;


    }
    // cout << "end compute" << endl ;
    return alpha ;
}

//met à jour alpha ainsi que les données Dtilde, Plast, sommeC (AVANT AJOUT DE J)
//arguments:
// c ; taille de C avant j
// Dtilde: ce qu'il reste à satisfaire par C (avant ajout de j)
// lastUsed: indice du plus petit élément dans C utilisé à la satisfaction de Dtilde (avant ajout de j)
// sommePC: somme des puissances des éléments utiles de C (sans j)
void Separation::updateAlpha(int & alpha, IloIntArray & C, IloInt c, double & Dtilde, double & lastUsed, double & sommePUsed, IloInt j) {

    Dtilde += getPmax(j) ;

    //Cas alpha nul: les éléments de Cbar suffisaient jusqu'à présent à satisfaire la demande
    if (alpha == 0) {

        if (Dtilde <= 0) {
            return;
        }
        else {
            lastUsed=0 ;
            alpha++;
            int Pcompare = 0 ;
            if (c > 0) {
                Pcompare = getPmax(C[0]) ;
            }
            sommePUsed = fmax(getPmax(j), Pcompare) ; // si alpha=1: seul la plus grosse unité de C est utilisée

            return;
        }
    }

    //Cas alpha non nul
    if (getPmax(j) <= getPmax(C[lastUsed])) {
        if (sommePUsed >= Dtilde) {
            return;
        }
        else {
            alpha++;
            lastUsed++;
            double Pcompare =0 ;
            if (lastUsed < c) {
                Pcompare = getPmax(C[lastUsed]) ;
            }
            sommePUsed += fmax(getPmax(j), Pcompare);//seg fault?

        }
    }
    else { //j est plus grand que le plus petit élément lastUsed
        sommePUsed += getPmax(j) - getPmax(C[lastUsed]) ; //mise à jour des alpha plus grands éléments
        if (sommePUsed >= Dtilde) { // s'ils suffisent à couvrir Dtilde alors alpha est inchangé
            return;
        }
        else {
            alpha++;
            sommePUsed += getPmax(C[lastUsed]) ;
            lastUsed++;
        }
    }

}

IloInt Separation::ComputeAlpha_i(IloInt t, IloInt i, const IloIntArray & C, IloInt sommeC) {
    IloInt c = C.getSize() ;
    IloInt alpha = 0 ;

    int sommeCbar = pb->getSommePmax() ;
    sommeCbar -= sommeC ;

    int Dadjust = pb->getD(t) - sommeCbar ;

    int j = 0 ;

    while ( (Dadjust > 0) && (j < c) ) {
        if (C[j]!=i) {
            alpha ++ ;
            Dadjust -= getPmax(C[j]) ;
        }
        j++ ;
    }

    if (Dadjust > 0) { //Si il reste une demande partielle à satisfaire après avoir ajouté toutes les unités sauf i c'est que i ne peut être éteinte.
        alpha=n; //Par convention, alpha_i(C) = n
    }
    return alpha ;
}

void Separation::ComputeBetas_i(IloIntArray & Betas, IloInt t0, IloInt t1, IloInt i, const IloIntArray & C, IloInt sommeC, IloInt alphaMax) {
    int sizeI = t1-t0+1 ;
    IloIntArray Alphas_i(env, sizeI) ;

    for (int t = 0 ; t < sizeI ; t++) {
        Alphas_i[t] = ComputeAlpha_i(t0+t, i, C, sommeC) - alphaMax ;
    }


    for (int t = 0 ; t < sizeI ; t++) {
        Betas[t]=0 ;

        int start = t - getl(i) + 1 ;
        for (int z= fmax(0,start) ; z <= t ; z++) {
            if (Alphas_i[z] > Betas[t]) {
                Betas[t] = Alphas_i[z] ;
            }
        }
        Betas[sizeI - 1] = Alphas_i[sizeI - 1] ;
    }

    cout << "alphas: "<< Alphas_i << endl ;
    cout << "alphaMax : " << alphaMax << endl ;
    cout << "li : " << getl(i) << endl;
}


//TODO : il faut utiliser la demande max
IloInt Separation::alphaMax_C(const IloIntArray & C, IloInt t0, IloInt t1, IloInt sommeC) {
    IloInt alpha_max = ComputeAlpha(t0, C, sommeC);
    for (int t=t0+1 ; t <= t1 ; t++) {
        int alpha = ComputeAlpha(t, C, sommeC) ;
        if (alpha > alpha_max) {
            alpha_max = alpha ;
        }
    }
    return alpha_max ;
}

IloBool Separation::AlphaOK(const IloIntArray & C, IloInt i, IloInt t0, IloInt t1, IloInt sommeC, IloInt alpha_max) {
    int alpha_i ;
    if (t0 < t1) {
        alpha_i = ComputeAlpha_i(getDl(t0, t1, getl(i)), i, C, sommeC) ;
    }
    int result = 0 ;
    if (t0 < t1) {
        result = (alpha_i >= alpha_max) ;
    }
    else {
        result = 1 ;
    }
    return result ;
}

IloBool Separation::iOK(IloInt i, IloInt t0, IloInt t1) {
    if (getL(i) < t1 - t0) {
        return 0;
    }
    return 1;
}

IloBool Separation::iStarts(IloInt i) {
    if (SommeU[i] == 0) {
        return 0;
    }
    return 1;
}

///////////////////////////////////////////////////////////



///////////// Fonctions annexes séparation ////////////////

void Separation::permut(IloIntArray & C) {
    int n = C.getSize() ;
    int p, q;
    C[0] = 0;
    // Tirage
    for ( p = 1; p <=n ; p++ ) {
        q = rand()%p+1;
        C[p-1] = C[q-1];
        C[q-1] = p-1;
    }
}

int Separation::insert(IloIntArray & C, IloInt i) {
    int place = 0 ;
    int c = C.getSize(); // TODO: passer la taille de C en argument pour éviter de la calculer
    C.add(i);
    while ( i > C[place] && place < c) {
        place++ ;
    }
    for (IloInt j = c ; j > place ; j--) {

        C[j] = C[j-1];
    }
    C[place] = i ;
    return place ;
}

void Separation::removeEnPlace(IloIntArray & C, IloInt c, IloInt j) {
    for (int k = j+1 ; k < c ; k++) {
        C[k-1]=C[k] ;
    }
}

void Separation::remove(IloIntArray & B, const IloIntArray & C, IloInt c, IloInt j) {
    //IloIntArray B = IloIntArray(env, c-1) ;

    for (int k = 0 ; k < j; k++) {
        B[k]=C[k] ;
    }

    for (int k = j+1 ; k < c ; k++) {
        B[k-1]=C[k] ;
    }
}

///////////////////////////////////////////////////////////


///////// Fonctions séparation "up-set" ///////////

int Separation::getS(int alpha, const IloIntArray & C, IloIntArray & S) {
    S = IloIntArray(env, 0);
    int sommeS = 0 ;
    int c = C.getSize() ;
    for (int j = c-1 ; j >= alpha - 1 ; j--) {
        insert(S, C[j]) ;
        sommeS += getPmax(C[j]) ;
    }
    return sommeS ;
}

int Separation::getMinimalCover(IloIntArray & C, IloInt t, IloInt sommeC) {

    int c = C.getSize() ;
    IloIntArray B(env,n); // le vecteur d'incidence de C, B[j] = (indice de j dans C) + 1. Si B[j] = 0 alors j n'appartient pas à C.

    for (int j=  0 ; j <n ; j++) {
        B[j] = 0 ;
    }

    for (int j = 0 ; j < c ; j++) {
        B[C[j]] =  j+1 ;
    }

    int alphaC = ComputeAlpha(t, C, sommeC) ;
    int alpha = alphaC ;
    int sommeCminus = sommeC ;

    int stop=0;

    cout << "ici" << endl;
    //On enlève les éléments de C dans l'ordre de leur contribution décroissante (indicesPureCost)
    while (!stop) {
        stop=1 ;
        for (int j = 0 ; (j < n) ; j++) {

            cout << "j: " << j << endl;
            cout << B[indicesPureCost[j]] << endl ;
            cout << "indics: " << indicesPureCost << endl ;
            cout << "B: " << B << endl ;
            cout << "C: " << C << endl ;

            if (B[indicesPureCost[j]] > 0) {

                cout << "C: " << C << endl ;
                C.remove(B[indicesPureCost[j]] - 1) ;
                cout << "C: " << C << endl ;

                int Pj = getPmax(indicesPureCost[j]) ;
                sommeCminus = sommeCminus - Pj ;
                cout << "somme: " << sommeCminus << endl ;

                alpha =  ComputeAlpha(t, C, sommeCminus) ;
                cout << "alpha: " << alpha << endl ;

                if (alpha==0) {
                    cout << "indice: " << indicesPureCost[j] << endl;
                    insert(C, indicesPureCost[j]);
                    sommeCminus += Pj ;
                    cout <<" C: " << C << endl ;
                }

                else {
                    cout << "on enlève : " << indicesPureCost[j] << endl ;
                    //mise à jour de l'ensemble B: si l'unité indicePureCost[k] a une Pmax plus petite que indicesPureCost[j] (indice plus grand)
                    // et indicesPureCost[k] est dans C, il faut décroître son indice dans C
                    B[indicesPureCost[j]] = 0 ;
                    for (int k = 0 ; k < n ; k++) {
                        if (indicesPureCost[k] > indicesPureCost[j] && B[indicesPureCost[k]] > 0) {
                            B[indicesPureCost[k]] -- ;
                        }
                    }
                    stop=0 ;
                }
                cout << "stop: " << stop << endl ;
                cout << endl ;
            }
        }
    }
    cout << "fin get" << endl;
    return sommeCminus;
}

void Separation::getStrong(IloIntArray & C, IloInt t, IloInt sommeC) {
    int c = C.getSize() ;
    int alphaC = ComputeAlpha(t, C, sommeC) ;
    int alpha = alphaC ;
    int sommeCminus = sommeC ;

    int stop=0 ;
    while (!stop) {
        int j1 = C[0] ;
        int i1 = C[0]+1;
        for (int k = 1 ; k < c ; k++) {
            if (C[k-1] + 1 == C[k]) {
                i1 = C[k] + 1;
            }
            else {
                k=c ;
            }
        }
        if (i1 < n)  {
            C.remove(0) ;
            sommeCminus -= getPmax(j1) ;

            int ind_i1= insert(C,i1);
            sommeCminus += getPmax(i1);
            alpha = ComputeAlpha(t, C, sommeCminus) ;

            if (alpha == 0) {
                stop=1 ;
                C.remove(ind_i1);
                insert(C, j1);
                sommeCminus += getPmax(j1) -  getPmax(i1);
            }
        }
        else {
            stop=1 ;
        }
    }
}

int Separation::getExt(IloIntArray & C) {
    int j1 = C[0];
    int count = 0 ;
    for (int i = 0 ; i < j1 ; i++) {
        insert(C, i);
        count++ ;
    }


    //on ajoute également les unités dont les indices sont supérieurs à j1 mais dont la Pmax = Pmax(j1)
    // C est trié par indice croissant (sachant que les indices sont triés par Pmax croissants)
    int c = C.getSize() ;
    int j2 = n-1; // si C est de taille 1 on cherche tout
    int keepOn = 1;
    int indice = 0;

    if (c > 1) {
        j2 = C[1] ;
    }


    while (keepOn && (indice < c)) {
        int j = j1+1 ;

        while ( (j < j2) && ( getPmax(j) == getPmax(j1) ) ) {

            insert(C, j);
            count++ ;
            j++ ;
        }

        if (getPmax(j2) != getPmax(C[0])) {
            keepOn = 0 ;
        }

        if (indice == c-1) {
            keepOn = 0 ;
        }
        else {
            j1 = j2 ;
            indice ++ ;
            j2=C[indice] ;
        }

    }

    return (count+1) ;
}

int Separation::getExt_(const IloIntArray & C, int c_size, IloIntArray & S, int & valS) {
    //Précondition : valS=val(C)

    S = IloIntArray(env, c_size) ;
    for (int i = 0 ; i < c_size ; i++) {
        S[i] = C[i] ;
    }

    int j1 = S[0];
    int count = 0 ;
    for (int i = 0 ; i < j1 ; i++) {
        insert(S, i);
        valS += coutW[i] ;
        count++ ;
    }


    //on ajoute également les unités dont les indices sont supérieurs à j1 mais dont la Pmax = Pmax(j1)
    // C est trié par indice croissant (sachant que les indices sont triés par Pmax croissants)
    /*  int c = S.getSize() ;
    int j2 = n-1; // si C est de taille 1 on cherche tout
    int keepOn = 1;
    int indice = 0;

    if (c > 1) {
        j2 = S[1] ;
    }


    while (keepOn && (indice < c)) {
        int j = j1+1 ;

        while ( (j < j2) && ( getPmax(j) == getPmax(j1) ) ) {

            insert(S, j);
                        valS += coutW[j] ;
            count++ ;
            j++ ;
        }

        if (getPmax(j2) != getPmax(S[0])) {
            keepOn = 0 ;
        }

        if (indice == c-1) {
            keepOn = 0 ;
        }
        else {
            j1 = j2 ;
            indice ++ ;
            j2=S[indice] ;
        }

    }
*/
    return (count+1) ;
}

int Separation::getFacet(int alpha, int t, const IloIntArray & C, IloIntArray & S) {
    int sommeS = getS(alpha, C, S) ;

    cout << "min cover:" << endl ;
    sommeS = getMinimalCover(S, t, sommeS) ;

    cout << "get strong:" << endl ;
    getStrong(S, t, sommeS) ;
    //int isFac= isFacet(S,t);
    //cout << "S: " << S << endl ;

     cout << "get ext:" << endl ;
    int alphaES = getExt(S);
    //cout << "E(S): " << S << endl ;
    //cout << endl ;
    return alphaES ;
}

bool Separation::isFacet(IloIntArray & S, IloInt t) {
    int s = S.getSize() ;
    IloIntArray T(env,0) ;
    int sommeT = 0 ;
    int firstInS = (S[0]==0) ;
    if (!firstInS) {
        insert(T, 0) ;
        sommeT += getPmax(0) ;
    }
    else {
        insert(T, S[0]) ;
        sommeT += getPmax(S[0]) ;
    }

    for (int i=2 ; i < s ; i++) {
        insert(T, S[i]) ;
        sommeT += getPmax(S[i]) ;
    }
    int alphaT = ComputeAlpha(t, T, sommeT) ;
    return (alphaT==0) ;
}

///////////////////////////////////////////////////////////






///////// Fonctions séparation "interval up-set" ///////////


//Enlève tous les éléments j!=i dans C tels que alpha_max(C\j) est inchangé et les conditions de validité toujours vérifiées
int Separation::getMinimal(IloIntArray & C, IloInt t0, IloInt t1, IloInt i, IloInt sommeC) {

    pb->quickSort(coutW,indicesPureCost, 0, n);

    int c = C.getSize() ;
    IloIntArray B(env,n); // le vecteur d'incidence de C, B[j] = indice dans C + 1. Si B[j] = 0 alors j n'appartient pas à C.

    for (int j=  0 ; j <n ; j++) {
        B[j] = 0 ;
    }
    for (int j = 0 ; j < c ; j++) {
        B[C[j]] =  j+1 ;
    }

    int alpha_max = alphaMax_C(C, t0, t1, sommeC) ;
    int sommeCminus = sommeC ;

    //On enlève les éléments de C dans l'ordre de leur contribution décroissante (indicesPureCost)
    for (int j = 0 ; j < n ; j++) {
        if (B[indicesPureCost[j]] > 0 && indicesPureCost[j] != i) {
            C.remove(B[indicesPureCost[j]] - 1) ;
            int Pj = getPmax(indicesPureCost[j]) ;
            sommeCminus = sommeCminus - Pj ;
            int alpha = alphaMax_C(C, t0, t1, sommeCminus) ;

            //Conditions pour rejeter indicesPureCost[j]
            bool Cond_1 = (alpha != alpha_max) || !AlphaOK(C, i, t0, t1, sommeCminus, alpha) ;
            if (Cond_1) {
                insert(C, indicesPureCost[j]);
                sommeCminus += Pj ;
            }
            else {
                //mise à jour de l'ensemble B: si l'unité indicePureCost[k] a une Pmax plus petite que indicesPureCost[j] (indice plus grand)
                // et indicesPureCost[k] est dans C, il faut décroître son indice dans C
                B[indicesPureCost[j]] = 0 ;
                for (int k = 1 ; k < n ; k++) {
                    if (indicesPureCost[k] > indicesPureCost[j] && B[indicesPureCost[k]] > 0) {
                        B[indicesPureCost[k]] -- ;
                    }
                }
                // cout << "C : " << C << endl ;
            }
        }
    }
    return alpha_max ;
}







////////// Inégalités violées ///////////

IloBool Separation::ICviolee(const IloIntArray & C, IloInt i, IloInt alpha, double valeur) {
    valeur -= alpha ;
    if (valeur < -eps) {
        //cout << "valeur violation: " << valeur << endl ;
        return 1 ;
    }
    else {
        return 0 ;
    }
}

IloBool Separation::Cviolee(const IloIntArray & C, IloInt alpha, double valeur) {
    valeur -= alpha ;
    if (valeur < -eps) {
        return 1 ;
    }
    else {
        return 0 ;
    }
}

////////////////////////////////


/// Séparations ///

//Séparation interval-up-set
IloInt Separation::separeGlouton(IloInt t0, IloInt t1, IloInt i, IloIntArray & C) {
    C = IloIntArray(env,0) ;
    C.add(i) ;
    int c_size = 1 ;
    int sommeC = getPmax(i) ;

    //Données utiles à la mise à jour de alpha
    double Dtilde = getDmax(t0, t1) - (sommeP - getPmax(i)) ; //la demande restante à satisfaire par C (Dtmax - somme des P sur Cbar)

    int alpha = (Dtilde >0) ;
    double lastUsed = 0; // la plus petite puissance dans C
    double sommePUsed =  alpha*getPmax(i)  ;


    int j = 0 ;
    int stop = 0 ;

    double valeur = coutZ[i] ; //somme des contributions des j != i + contribution i

    while ( !stop && (c_size < n) ) {
        if ( (indices[j] != i) ) {

            //Mises à jour
            updateAlpha(alpha, C, c_size, Dtilde, lastUsed, sommePUsed, indices[j]) ;
            //cout << "alpha : " << alpha << endl ;

            //cout << "alpha réel : " << alphaMax_C(C, t0, t1, sommeC)  << endl ;

            //insertion de j = argmin ...
            insert(C, indices[j]) ;

            sommeC += getPmax(indices[j]) ;
            //alpha = alphaMax_C(C, t0, t1, sommeC) ;
            valeur += coutW[indices[j]] ;
            c_size++ ;
            j++ ;

            stop = (  AlphaOK(C, i, t0, t1, sommeC, alpha) &&  ICviolee(C, i, alpha, valeur) ) ;
        }
        else {
            j++;
        }
    }

    if (alpha*stop > 0) {
        alpha = getMinimal(C, t0, t1, i, sommeC)  ;

        /* cout << "C: " << C << endl ;
        cout << "alpha: " << alpha << endl ;
        cout << "t0, t1: " << t0 << ", " << t1 << endl ;

        int csize=C.getSize() ;
        int sommeCmin=0 ;
        for (int k=0 ; k < csize ; k++) {
            sommeCmin += getPmax(C[k]);
        }
        int alphaReal = ComputeAlpha(getDmax(t0, t1), C, sommeCmin) ;
        cout << "alphaReal : " << alphaReal << endl;
        cout << endl ;*/
    }
    alpha = alpha*stop ;

    return alpha ;
}

IloInt Separation::separeRandom(IloInt t0, IloInt t1, IloInt i, IloIntArray & C) {
    C = IloIntArray(env,0) ;

    int c_size = 0 ;
    int sommeC = 0 ;

    //Données utiles à la mise à jour de alpha
    double Dtilde = getDmax(t0, t1) - sommeP ; //la demande restante à satisfaire par C (Dtmax - somme des P sur Cbar)

    int alpha = (Dtilde >0) ;
    double lastUsed = 0; // la plus petite puissance dans C
    double sommePUsed = 0  ;


    int j = 0 ;
    int stop = 0 ;

    double valeur = coutZ[i] ; //somme des contributions des j != i + contribution i
    int sizeC = (rand()%(n-1)) + 2 ;

    while ( !stop && (c_size < n) && (j<n) ) {

        int onprend = 0;
        int chiffre = rand()%1000;
        if (chiffre*n < sizeC*1000) {
            onprend=1 ;
        }

        if ( j==i ) {
            onprend=1 ;
        }

        if ( onprend ) {

            //Mises à jour
            updateAlpha(alpha, C, c_size, Dtilde, lastUsed, sommePUsed, j) ;
            //cout << "alpha : " << alpha << endl ;

            //cout << "alpha réel : " << alphaMax_C(C, t0, t1, sommeC)  << endl ;

            //insertion de j = argmin ...
            C.add(j) ;

            sommeC += getPmax(j) ;
            //alpha = alphaMax_C(C, t0, t1, sommeC) ;
            valeur += coutW[j] ;
            c_size++ ;


            stop = (  AlphaOK(C, i, t0, t1, sommeC, alpha) &&  ICviolee(C, i, alpha, valeur) ) ;
        }
        j++ ;

    }

    if (alpha*stop > 0) {
        int sommeC2 = sommeC ;
        alpha = getMinimal(C, t0, t1, i, sommeC2)  ;

        cout << "C: " << C << endl;
        cout << "alpha: " << alpha << endl ;
    }
    alpha = alpha*stop ;

    return alpha ;
}



// Séparation up-set
IloInt Separation::separeCover(IloInt t, IloIntArray & C) {
    C = IloIntArray(env,0) ;
    int c_size = 0 ;
    int sommeC = 0 ;

    //Données utiles à la mise à jour de alpha
    double Dtilde = getDmax(t, t) - sommeP  ; //la demande restante à satisfaire par C (Dtmax - somme des P sur Cbar)

    int alpha = (Dtilde >0) ;
    double lastUsed = 0; // la plus petite puissance dans C
    double sommePUsed =  0  ;


    int j = 0 ;
    int stop = 0 ;

    double valeur=0 ;

    while ( !stop && (c_size < n) ) {

        //Mises à jour
        updateAlpha(alpha, C, c_size, Dtilde, lastUsed, sommePUsed, indices[j]) ;

        //insertion de j = argmin ...
        insert(C, indices[j]) ;

        //Mises à jour

        sommeC += getPmax(indices[j]) ;
        // alpha = ComputeAlpha(t, C, sommeC) ;
        valeur+=coutW[indices[j]] ;
        c_size++ ;
        j++ ;
        stop = ( Cviolee(C, alpha, valeur) ) ;
    }
    if (alpha*stop > 0) {
        IloIntArray S(env, 0) ;

        int alphaES = getFacet(alpha, t, C, S) ;

        C=S ;
        alpha = alphaES;
    }
    alpha = alpha*stop ;
    return alpha ;
}




IloInt Separation::separeLetchford(IloInt t, IloIntArray & C) {

    int alphaES ;

    //Données utiles à la mise à jour de alpha

    int alpha = 0 ;
    int found = 0 ;
    IloIntArray S(env, 0) ;

    C = IloIntArray(env,0) ;

    int c_size = 0 ;
    int sommeC = 0 ;;
    double valeur=0 ;

    int j=0 ;
    int count=0 ;
    double a_star = getDmax(t,t) ;

    while (!found && count < n) {

        S = IloIntArray(env,0) ;
        int valS ;
        count++ ;

        if (getPmax(indices[j]) <= a_star ) {

            //insertion de j = argmin ...
            insert(C, indices[j]) ;
            sommeC += getPmax(indices[j]) ;
            valeur+=coutW[indices[j]] ;
            c_size++ ;

            alpha = isUpSet(t, C, sommeC) ;

            //int alphaC=ComputeAlpha(t,C,sommeC) ;

            /*cout << "C: " << C << endl ;
                                cout << "alpha: " << alpha << endl ;
                                cout << "alphaC: " << alpha << endl ;
                                */
            if (alpha > 0) {

                valS = valeur ;
                alphaES = getExt_(C, c_size, S, valS) ;
                found = Cviolee(S, alphaES, valS) ;
            }

            if (found) {



            }

            if (!found && alpha > 0) {
                //on retire k de C
                a_star = getPmax(C[0]) ;

                sommeC -= a_star ;
                valeur -= coutW[C[0]] ;
                removeEnPlace(C, c_size, 0) ;
                c_size-- ;
            }
        }

        j++ ;
    }

    if (found) {
        C=S ;
        alpha = alphaES;
    }
    alpha = alpha*found ;

    return alpha ;
}






int Separation::SepareSCIP(list<int>* C_list, IloInt i, IloInt t0, IloInt t1)  {
    IloIntArray C(env, 0) ;

    int alpha = separeGlouton(t0, t1, i, C) ;

    if (alpha > 0) {
        cout << "i:" << i << endl;
        cout << "interval: " << t0 << ", " << t1 << endl ;
        cout << "alpha: " << alpha << endl ;
        cout << "C: " << C << endl ;
        for (int j=0 ; j < C.getSize() ; j++) {
            if (C[j] != i) {
            C_list->push_back(pb->getTri(C[j])) ;
            }
        }
        return alpha ;
    }
    return 0 ;
}




void Separation::quickSort(IloNumArray const & ordre, IloIntArray & indices, IloInt p, IloInt q) {
    IloInt r;
    if(p<q)
    {
        r=partition(ordre, indices, p,q);
        quickSort(ordre, indices,p,r);
        quickSort(ordre, indices,r+1,q);
    }
}

IloInt Separation::partition(IloNumArray const & ordre, IloIntArray & indices, IloInt p, IloInt q)
{
    IloInt x= indices[p];
    IloInt i=p;
    IloInt j;

    for(j=p+1; j<q; j++)
    {
        if(ordre[indices[j]]<= ordre[x])
        {
            i=i+1;
            swap(indices[i],indices[j]);
        }

    }

    swap(indices[i],indices[p]);
    return i;
}

