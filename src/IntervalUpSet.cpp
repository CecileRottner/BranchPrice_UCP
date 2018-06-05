#include "IntervalUpSet.h"
#include "InstanceUCP.h"

#include <ilcplex/ilocplex.h>
#include <string>

using namespace std ;

Separation::Separation(IloEnv envir, InstanceUCP* inst, const IloBoolVarArray & xx, const IloBoolVarArray & uu, IloNum epsi) :
    env (envir),
    pb (inst),
    x (xx),
    u (uu),
    eps (epsi) {
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
    return pb->getL(i) ;
}

IloInt Separation::getl(IloInt i) {
    return pb->getl(i) ;
}

IloInt Separation::getExiste_i(IloInt t0, IloInt t1, IloInt l) {
    return existe_i[t1 - t0 - 1][l-1] ;
}

int Separation::getLmax() {
    return Lmax ;
}

IloInt Separation::getPmax(IloInt i) {
    return pb->getPmax(i) ;
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

void Separation::computeCosts(IloInt t0, IloInt t1, const IloNumArray & xx, const IloNumArray & uu) {
    //Calcul de coutW et coutZ pour intervalle [t0, t1] et (xx,uu) donnés

    //coutW
    for (IloInt j = 0 ; j < n ; j++) {
        coutW[j] = xx[T*j + t0] ;
        for (IloInt k = t0+1 ; k <= t1 ; k++) {
            coutW[j] += uu[j*T + k] ;
        }
    }

    //coutZ
    for (IloInt j = 0 ; j < n ; j++) {
        coutZ[j] = xx[T*j + t1] ;
        for (IloInt k = t0+1 ; k <= t1 ; k++) {
            coutZ[j] -= uu[j*T + k] ;
        }
    }

    //Tri du vecteur indicesPureCost est fait dans les fonction getMinimal, donc seulement si on en a besoin
    if (t0==t1) {
        pb->quickSort(coutW,indicesPureCost, 0, n);
    }
}

void Separation::computeWeightedCosts(IloInt t0, IloInt t1, const IloNumArray & xx, const IloNumArray & uu) {

    //Calcul
    for (IloInt j = 0 ; j < n ; j++) {
        WeightedCost[j] = xx[T*j + t0] ;
        for (IloInt k = t0+1 ; k <= t1 ; k++) {
            WeightedCost[j] += uu[j*T + k] ;
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

    //On enlève les éléments de C dans l'ordre de leur contribution décroissante (indicesPureCost)
    while (!stop) {
        stop=1 ;
        for (int j = 0 ; (j < n) ; j++) {

            if (B[indicesPureCost[j]] > 0) {

                C.remove(B[indicesPureCost[j]] - 1) ;

                int Pj = getPmax(indicesPureCost[j]) ;
                sommeCminus = sommeCminus - Pj ;

                alpha =  ComputeAlpha(t, C, sommeCminus) ;

                if (alpha==0) {
                    insert(C, indicesPureCost[j]);
                    sommeCminus += Pj ;
                }

                else {
                    //cout << "on enlève : " << indicesPureCost[j] << endl ;
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
            }
        }
    }
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
    sommeS = getMinimalCover(S, t, sommeS) ;
    getStrong(S, t, sommeS) ;
    //int isFac= isFacet(S,t);
    //cout << "S: " << S << endl ;
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

            int alphaC=ComputeAlpha(t,C,sommeC) ;

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




void Separation::constructInequality(IloRange & cons, IloInt alpha, const IloIntArray & C, IloInt t0, IloInt t1, IloInt i, IloInt methode) {

    int sommeC = 0 ;
    for (int j = 0 ; j < C.getSize() ; j++) {
        sommeC +=getPmax(C[j]) ;
    }

    cout << "C: " << C << ", dont unité i=" << i <<endl ;
    cout << "Intervalle: " << t0 << ", " << t1 << endl;
    cout << "alpha: " << alpha << endl ;



    //Contributions
    double contribJ = 0 ;
    for (int j = 0 ; j < C.getSize() ; j++) {
        if (C[j] != i) {
            contribJ += coutW[C[j]]  ;
        }
    }
    contribJ += coutZ[i] ;

    int c  = C.getSize() ;
    IloIntArray Betas(env, t1 - t0 + 1) ;
    for (int t=0 ; t < t1 - t0 + 1 ; t++) {
        Betas[t] = 0 ;
    }

    //Calcul des bêtas
    if (t0 < t1 && methode==3) {

        ComputeBetas_i(Betas, t0, t1, i, C, sommeC, alpha) ;
        cout << "Betas : " << Betas << endl;
    }

    if (t0==t1) {
        cons.setLB(alpha) ;
    }
    else {
        cons.setLB(alpha + Betas[t1-t0]) ;
    }
    IloExpr expr(env) ;

    expr += x[i*T + t1] ;
    for (int j=0 ; j < c ; j++) {
        if (C[j] != i) {
            expr += x[C[j]*T + t0] ;

        }
        for (IloInt k=t0+1 ; k <= t1 ; k++)  {
            if (C[j] != i) {
                expr += u[C[j]*T + k];
            }
            else {
                expr -= (Betas[k-1-t0] + 1)*u[C[j]*T + k];
            }
        }
    }
    if (t0 < t1) {
        expr += Betas[t1-t0]*x[i*T + t1] ;
    }
    cons.setExpr(expr);
    expr.end() ;

}


void Separation::Separe(IloRange & cons, IloInt i, IloInt t0, IloInt t1, IloInt methode)  {
    IloIntArray C(env, 0) ;
    int alpha=0 ;
    if (methode==1) {
        alpha = separeCover(t0,C);
        i=C[0];
    }
    if (methode==5) {
        alpha = separeLetchford(t0,C);
        if (C.getSize() > 0) {
            i=C[0];
        }
        else {
            i=0 ;
        }
    }
    if (methode==2 || methode==3) {
        alpha = separeGlouton(t0, t1, i, C) ;
    }
    if (methode==4) {
        alpha = separeRandom(t0, t1, i, C) ;
    }
    if (alpha > 0) {
        constructInequality(cons, alpha, C, t0, t1, i, methode) ;
    }
    C.end() ;
}





////////////////// Séparation exacte up-set ////////////////////

double Separation::resoutPLCover(IloInt t, IloInt alpha, IloIntArray & C, IloInt & sommeC, IloInt & alphaC) {
    //retourne -1 si pas de solution ou si la solution trouvée n'est pas violée

    sommeC = 0 ;
    alphaC = 0 ;
    IloEnv envPL ;
    IloBoolVarArray y(envPL, n) ;
    IloBoolVarArray z(envPL, n) ;

    IloModel model(envPL) ;

    IloExpr obj(envPL) ;

    for (IloInt j = 0 ; j < n ; j++) {
        obj += coutW[j]*y[j];
    }

    model.add(IloMinimize(envPL, obj)) ;
    obj.end();

    IloExpr sumY(envPL);
    for (IloInt j = 0 ; j < n ; j++) {
        sumY+=y[j] ;
    }
    model.add(sumY >= alpha) ;


    IloExpr expr0(envPL) ;

    for (IloInt j = 0 ; j < n ; j++) {
        expr0 += getPmax(j)*(1 - y[j]) ;
    }
    expr0+= -pb->getD(t) ;
    model.add(expr0 < 0) ;

    int* v= new int[n] ;
    for (int j =0 ; j <n ; j++) {
        v[j] = 0 ;
    }
    addSubsets(envPL, y, z, expr0, model, n, alpha-1, v, 0);


    delete[] v;

    model.add(expr0 < 0) ;
    expr0.end() ;


    IloCplex cplex(model) ;
    cplex.setOut(env.getNullStream());

    int solve = cplex.solve() ;

    if (solve == 1)  {
        double value = cplex.getObjValue() ;

        IloNumArray Y(envPL, n) ;
        cplex.getValues(Y, y) ;


        for (IloInt j = 0 ; j < n ; j++) {
            if (Y[j] > 0.99) {
                C.add(j) ;
                sommeC+= getPmax(j) ;
            }
        }

        alphaC=ComputeAlpha(t,C,sommeC) ;

        if (value >= alphaC-eps) { //vérifier si alpha n'est pas supérieur à 1 ?
            value=-1;
        }

        envPL.end();
        return value ;
    }

    else {
        envPL.end();
        return -1 ;
    }
}				








/////////////////Séparation exacte Interval upset //////////////////////////
//Séparation exacte

/*void Separation::Separe(IloRange & cons, const IloNumArray & xx, const IloNumArray & uu, IloInt t0, IloInt t1)  {
        computeCosts(t0,t1,xx,uu) ;

        IloInt i ;
        IloIntArray C(env, 0) ;

        int alpha = separeExact(xx, uu, t0, t1, i, C) ;
        if (alpha>0) {
                constructInequality(cons, alpha, C, t0, t1, i) ;

                }
                }*/

IloInt Separation::separeExact(IloInt t0, IloInt t1, IloInt & i, IloIntArray & C) {
    /*  int alpha = 0 ;
         int stop ;
         int amax = 1 ;
         for (int l = 1 ; l <= lmax ; l++) {
                 cout << "---- l :" << l << "----" << endl ;
                 stop = 0 ;
                 if (getExiste_i(t0, t1, l)) {
                         alpha = 1 ;
                         while (!stop) {
                                 cout << "----- alpha : " << alpha << "-----" << endl ;
                                 double result = resoutPL(alpha, t0, t1, l, C, i) ;
                                 int sizeC = C.getSize() ;
                                 int sommeC = 0 ;
                                 for (int j=0 ; j < sizeC ; j++) {
                                         sommeC += getPmax(C[j]) ;
}
int alphaC = ComputeAlpha(getIndDmax(t0,t1), C, sommeC);
cout << "Résultat :" << endl ;
cout << "C : " << C << endl ;
cout << "alpha_max(C) : " << alphaC << endl ;
cout << "valeur : " << result << endl ;
cout << endl ;
if ((result > 0) && (ICviolee(C,i, alpha))) {
        stop=1;
}
else {
        C.end();
        C = IloIntArray(env, 0) ;

        alpha = fmax(alpha+1, ceil(result)) ;


        if (alpha > amax) {
                alpha=0 ;
                stop=1 ;
}
}
}
}
}
return alpha ;*/
    return 0 ;

}



void Separation::addSubsets(IloEnv envPL, IloBoolVarArray y, IloBoolVarArray z, IloExpr expr_i, IloModel model, IloInt N, IloInt alpha, int* v, IloBool i) {
    if (alpha==0) {
        IloExpr expr(envPL) ;
        expr += expr_i ;
        for (int j=0 ; j < n ; j++) {
            if (i == 0) {
                expr += v[j]*getPmax(j)*y[j] ;
            }
            else {
                expr += v[j]*getPmax(j)*(y[j] + z[j]) ;
            }

        }
        model.add(expr < 0) ;
    }
    else if (N<alpha) {
    }
    else {
        addSubsets(envPL, y,z, expr_i, model,N-1, alpha, v, i) ;

        v[N-1] = 1 ;
        addSubsets(envPL, y, z,expr_i, model, N-1, alpha-1, v, i) ;
        v[N-1] = 0 ;
    }

}




double Separation::resoutPL(int alpha, IloInt t0, IloInt t1, IloInt l, IloIntArray & C, IloInt & i) {

    IloEnv envPL ;
    IloBoolVarArray y(envPL, n) ;
    IloBoolVarArray z(envPL, n) ;

    IloModel model(envPL) ;

    IloExpr obj(envPL) ;

    for (IloInt j = 0 ; j < n ; j++) {
        obj += coutW[j]*y[j] + coutZ[j]*z[j];
    }

    model.add(IloMinimize(envPL, obj)) ;
    obj.end();


    for (IloInt j = 0 ; j < n ; j++) {
        model.add(y[j] + z[j] <= 1) ;
    }

    IloExpr sumZ(envPL);
    for (IloInt j = 0 ; j < n ; j++) {
        if (t1 - t0 > getL(j)) {
            model.add(z[j] == 0) ;
        }
        if ( l != getl(j) ) {
            model.add(z[j] == 0) ;
        }
        sumZ+=z[j] ;
    }
    model.add(sumZ == 1) ;

    IloExpr sumY(envPL);
    for (IloInt j = 0 ; j < n ; j++) {
        sumY+=y[j] ;
    }
    model.add(sumY >= alpha) ;

    IloExpr expr0(envPL) ;
    IloExpr expr1(envPL) ;
    for (IloInt j = 0 ; j < n ; j++) {
        expr0 += getPmax(j)*(1 - y[j] - z[j]) ;
        expr1 += getPmax(j)*(1 - y[j] - z[j]) ;
    }
    expr0+= -pb->getD(getDl(t0,t1,l)) ;
    expr1+= -getDmax(t0, t1) ;

    cout << "Ajout des contraintes exponentielles..." << endl ;

    /*int* v= new int[n] ;
        for (int j =0 ; j <n ; j++) {
                v[j] = 0 ;
}
addSubsets(envPL, y, z, expr0, model, n, alpha-1, v, 0);
addSubsets(envPL, y, z, expr0, model, n, alpha-1, v, 1);

delete[] v;*/

    //Pour alpha<=4

    if (alpha == 1) {
        IloExpr cons0(envPL);
        IloExpr cons1(envPL);
        cons0 += expr0 ;
        cons1 +=expr1 ;

        model.add(cons0 < 0) ;
        model.add(cons1 < 0) ;
        cons0.end() ;
        cons1.end();
    }

    else {
        for (int i1 = 0 ; i1 < n ; i1++) {

            if (alpha == 2) {
                IloExpr cons0(envPL);
                IloExpr cons1(envPL);
                cons0 += expr0 ;
                cons1 +=expr1 ;

                cons0 += y[i1]*pb->getPmax(i1) ;
                cons1 += (y[i1] + z[i1])*pb->getPmax(i1) ;

                model.add(cons0 < 0) ;
                model.add(cons1 < 0) ;
                cons0.end() ;
                cons1.end();
            }

            else {
                for (int i2 = i1+1 ; i2 < n ; i2++) {

                    if (alpha == 3) {
                        IloExpr cons0(envPL);
                        IloExpr cons1(envPL);
                        cons0 += expr0 ;
                        cons1 +=expr1 ;

                        cons0 += y[i1]*pb->getPmax(i1) ;
                        cons1 += (y[i1] + z[i1])*pb->getPmax(i1) ;

                        cons0 += y[i2]*pb->getPmax(i2) ;
                        cons1 += (y[i2] + z[i2])*pb->getPmax(i2) ;

                        model.add(cons0 < 0) ;
                        model.add(cons1 < 0) ;
                        cons0.end() ;
                        cons1.end();
                    }

                    else {
                        for (int i3 = i2+1 ; i3 < n ; i3++) {

                            if (alpha == 4) {
                                IloExpr cons0(envPL);
                                IloExpr cons1(envPL);
                                cons0 += expr0 ;
                                cons1 +=expr1 ;

                                cons0 += y[i1]*pb->getPmax(i1) ;
                                cons1 += (y[i1] + z[i1])*pb->getPmax(i1) ;

                                cons0 += y[i2]*pb->getPmax(i2) ;
                                cons1 += (y[i2] + z[i2])*pb->getPmax(i2) ;

                                cons0 += y[i3]*pb->getPmax(i3) ;
                                cons1 += (y[i3] + z[i3])*pb->getPmax(i3) ;

                                model.add(cons0 < 0) ;
                                model.add(cons1 < 0) ;
                                cons0.end() ;
                                cons1.end();
                            }

                            else {
                                for (int i4 = i3+1 ; i4 < n ; i4++) {

                                    if (alpha == 5) {
                                        IloExpr cons0(envPL);
                                        IloExpr cons1(envPL);
                                        cons0 += expr0 ;
                                        cons1 +=expr1 ;

                                        cons0 += y[i1]*pb->getPmax(i1) ;
                                        cons1 += (y[i1] + z[i1])*pb->getPmax(i1) ;

                                        cons0 += y[i2]*pb->getPmax(i2) ;
                                        cons1 += (y[i2] + z[i2])*pb->getPmax(i2) ;

                                        cons0 += y[i3]*pb->getPmax(i3) ;
                                        cons1 += (y[i3] + z[i3])*pb->getPmax(i3) ;

                                        cons0 += y[i4]*pb->getPmax(i4) ;
                                        cons1 += (y[i4] + z[i4])*pb->getPmax(i4) ;


                                        model.add(cons0 < 0) ;
                                        model.add(cons1 < 0) ;
                                        cons0.end() ;
                                        cons1.end();
                                    }

                                    else {

                                        for (int i5 = i4+1 ; i5 < n ; i5++) {

                                            if (alpha == 6) {
                                                IloExpr cons0(envPL);
                                                IloExpr cons1(envPL);
                                                cons0 += expr0 ;
                                                cons1 +=expr1 ;

                                                cons0 += y[i1]*pb->getPmax(i1) ;
                                                cons1 += (y[i1] + z[i1])*pb->getPmax(i1) ;

                                                cons0 += y[i2]*pb->getPmax(i2) ;
                                                cons1 += (y[i2] + z[i2])*pb->getPmax(i2) ;

                                                cons0 += y[i3]*pb->getPmax(i3) ;
                                                cons1 += (y[i3] + z[i3])*pb->getPmax(i3) ;

                                                cons0 += y[i4]*pb->getPmax(i4) ;
                                                cons1 += (y[i4] + z[i4])*pb->getPmax(i4) ;


                                                cons0 += y[i5]*pb->getPmax(i5) ;
                                                cons1 += (y[i5] + z[i5])*pb->getPmax(i5) ;

                                                model.add(cons0 < 0) ;
                                                model.add(cons1 < 0) ;
                                                cons0.end() ;
                                                cons1.end();
                                            }

                                            else {

                                                for (int i6 = i5+1 ; i6 < n ; i6++) {

                                                    if (alpha == 7) {
                                                        IloExpr cons0(envPL);
                                                        IloExpr cons1(envPL);
                                                        cons0 += expr0 ;
                                                        cons1 +=expr1 ;

                                                        cons0 += y[i1]*pb->getPmax(i1) ;
                                                        cons1 += (y[i1] + z[i1])*pb->getPmax(i1) ;

                                                        cons0 += y[i2]*pb->getPmax(i2) ;
                                                        cons1 += (y[i2] + z[i2])*pb->getPmax(i2) ;

                                                        cons0 += y[i3]*pb->getPmax(i3) ;
                                                        cons1 += (y[i3] + z[i3])*pb->getPmax(i3) ;

                                                        cons0 += y[i4]*pb->getPmax(i4) ;
                                                        cons1 += (y[i4] + z[i4])*pb->getPmax(i4) ;

                                                        cons0 += y[i5]*pb->getPmax(i5) ;
                                                        cons1 += (y[i5] + z[i5])*pb->getPmax(i5) ;

                                                        cons0 += y[i6]*pb->getPmax(i6) ;
                                                        cons1 += (y[i6] + z[i6])*pb->getPmax(i6) ;

                                                        model.add(cons0 < 0) ;
                                                        model.add(cons1 < 0) ;
                                                        cons0.end() ;
                                                        cons1.end();
                                                    }

                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                }
            }

        }
    }



    cout << "fin ajout" << endl;
    //Pour alpha=2
    /*for (IloInt j = 0 ; j < n ; j++) {
                IloExpr expr0_j(envPL) ;
                expr0_j += expr0 ;
                expr0_j += getPmax(j)*y[j];
                model.add(expr0_j < 0) ;
                expr0_j.end() ;
}

for (IloInt j = 0 ; j < n ; j++) {
        IloExpr expr1_j(envPL) ;
        expr1_j += expr1 ;
        expr1_j += getPmax(j)*(y[j] + z[j]);
        model.add(expr1_j < 0) ;
        expr1_j.end() ;
}*/

    IloCplex cplex(model) ;
    cplex.setOut(env.getNullStream());

    cout << "Résolution..." << endl ;
    int solve = cplex.solve() ;
    cout << "Résolu" << endl ;



    if (solve == 1)  {
        double value = cplex.getObjValue() ;

        IloNumArray Y(envPL, n) ;
        cplex.getValues(Y, y) ;

        IloNumArray Z(envPL,n) ;
        cplex.getValues(Z,z) ;


        for (IloInt j = 0 ; j < n ; j++) {
            if (Z[j] > 0.99) {
                C.add(j) ;
                i = j ;
            }
            if (Y[j] > 0.99) {
                C.add(j) ;
            }
        }
        envPL.end();
        return value ;
    }

    else {
        envPL.end();
        return -1 ;
    }
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

