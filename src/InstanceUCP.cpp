#include "InstanceUCP.h"

#include <ctime>
#include <stdlib.h> // use rand
#include <fstream> // lire fichier de données

using namespace std ;


InstanceUCP::InstanceUCP(IloEnv envir, const char* file) {
    env = envir ;
    Lecture(file) ;
    Initialise() ;
}




void InstanceUCP::Initialise() {

    SommePmax = 0 ;
    for (int j = 0 ; j <n ; j++) {
        SommePmax += Pmax[j] ;
    }


    /////////////////////////////////////////////////////////////////
    ////////    Vecteur trié: unités par Pmax décroissants   ////////
    /////////////////////////////////////////////////////////////////

    Tri = IloIntArray(env, n);
    IloNumArray ordrePmax = IloNumArray(env, n);

    for (IloInt j = 0 ; j <n ; j++ ) {
        Tri[j] = j ;
        ordrePmax[j] = 1/Pmax[j] ;
    }

    quickSort(ordrePmax, Tri, 0, n);

    /////////////////////////////
    ////////    Sites    ////////
    /////////////////////////////

    firstUnitofSite = IloIntArray(env, S) ;
    int site=0 ;
    int unit=0;

    while (site < S) {
        while (firstOfSite[unit] == 0) {
            unit++ ;
        }
        firstUnitofSite[site]= unit ;
        site++ ;
        unit++ ;
    }

    cout << "fisrt unit of each site: " << firstUnitofSite << endl ;

    SiteOf = IloIntArray(env, n) ;
    site=-1 ;

    for (int j=0 ; j <n ; j++) {
        if (firstOfSite[j]) {
            site++;
        }
        SiteOf[j]=site ;
    }

    /////////////////////////////
    //////    Symétries   ///////
    /////////////////////////////

    nbG=0 ;

    for (int i = 0 ; i <n ; i++) {
        if (First[i]) {
            nbG++;
        }
    }

    FirstG = IloIntArray(env, nbG) ;
    LastG = IloIntArray(env, nbG) ;

    Group = IloIntArray(env, n) ;

    int nb2 =0 ;
    for (int i = 0 ; i <n ; i++) {
        if (First[i]) {
            FirstG[nb2] = i ;

        }
        if (Last[i]) {
            LastG[nb2] = i ;
            nb2++ ;
        }
    }

    cout << "First: " << First << endl ;
    cout << "FirstG: " << FirstG << endl ;




    SizeG = IloIntArray(env, nbG) ;

    repartition_tailles = IloIntArray(env, n+1) ;
    for (int i=0 ; i <n ; i++) {
        repartition_tailles[i] = 0 ;
    }

    MaxSize = 0 ;
    MeanSize = 0 ;
    nbG2 = 0 ;
    int groupSize = 0 ;
    int group_ind=0 ;

    for (int i = 0 ; i <n ; i++) {
        if (!Last[i]) {
            groupSize ++ ;
            if (First[i]) {
                nbG2++ ;
            }
        }
        else {
            groupSize++ ;
            repartition_tailles[groupSize]++ ;
            SizeG[group_ind] = groupSize ;

            if (!First[i]) {
                MeanSize+=groupSize ;
                if (MaxSize < groupSize) {
                    MaxSize = groupSize ;
                }
            }
            groupSize = 0 ;
            group_ind++;
        }
    }

    MeanSize = MeanSize / nbG2;

    nb2=0 ;

    for (int i = 0 ; i <n ; i++) {
        Group[i] = nb2 ;
        if (Last[i]) {
            nb2++ ;
        }
    }




    ///////////////////////////////////////////////////
    //////    Indicateurs pour le branchement   ///////
    ///////////////////////////////////////////////////

    //Tri des unités et des pas de temps
    IloNumArray ordre = IloNumArray(env, nbG);

    for (IloInt j = 0 ; j <nbG ; j++ ) {
        int elem = FirstG[j];
        ordre[j] = 1/Pmax[elem] ;
    }


    IloNumArray invDemande = IloNumArray(env, T);

    for (IloInt j = 0 ; j < T ; j++ ) {
        ordreT[j] = j ;
        invDemande[j] = 1/D[j] ;
    }

   // quickSort(invDemande, ordreT, 0, T) ; // ordreT trie les t par demande décroissante
    //cout << "ordreT : " << ordreT << endl ;



    ///// dérivée de la demande (pour le branchement) /////
    isIncreasing = IloIntArray(env, T) ;

    ordre_ratio = IloIntArray(env, n) ;

    /*for (int t=0 ; t < T ; t++) {
        int mean_next_k = 0;
        int k=5 ;
        for (int s=fmin(T-1,t+1) ; s <= fmin(T-1,t+k) ; s++) {
            mean_next_k += D[s] ;
        }
        mean_next_k /= k ;
        if (mean_next_k > D[t]) {
            isIncreasing[t] = 1 ;
        }
        else {
            isIncreasing[t] = 0 ;
        }
    }



    Dmin = SommePmax ;
    int Dmax1 = 0 ;
    int Dmax2 = 0 ;

    int t=0 ;
    while (D[t] > Dmax1) {
        tmax1=t ;
        Dmax1 = D[t] ;
        t++ ;
    }

    cout << "tmax1 : " << tmax1 << endl ;

    cout << "t : " << t << endl ;
    cout << "D[t], Dmin" << D[t] << ", " << Dmin << endl ;
    while (D[t] < Dmin) {
        tmin=t ;
        Dmin = D[t] ;
        t++ ;
    }

    cout << "tmin : " << tmin << endl ;
    while (D[t] > Dmax2) {
        tmax2=t ;
        Dmax2 = D[t] ;
        t++ ;
    }

    Dmax = fmax(Dmax1, Dmax2) ;

    cout << "Dmin, Dmax : " << Dmin << ", " << Dmax << endl;


    //Tri des unités selon le ratio

    IloNumArray ratio = IloNumArray(env, n);

    for (IloInt j = 0 ; j <n ; j++ ) {
        ordre_ratio[j] = j ;
        ratio[j] = (Pmax[j]*cp[j] + cf[j])/Pmax[j] ;
    }

    quickSort(ratio,ordre_ratio, 0, n);


    cout << "ici" << endl ;
    //unités de base et de pic

    //base
    nbase=0 ;
    int D = Dmin ;
    while (D > 0 && nbase < n) {
        D -= Pmax[nbase] ;
        nbase++ ;
    }

    nbase-- ;

    //pic

    np=nbase+1 ;
    D = Dmax - Dmin ;
    while (D > 0 && np < n) {
        D += Pmax[np] ;
        np++ ;
    }*/

}

void InstanceUCP::Lecture(const char* file) {
    //Lecture de n et de T
    ifstream fichier(file, ios::in);

    string nom = "";
    fichier >> nom;

    while(nom!="="){
        fichier >> nom;
    }
    nom = "";
    fichier >> n;

    while(nom!="="){
        fichier >> nom;
    }
    nom = "";
    fichier >> T;



    //Initialisation des vecteurs de taille n et T

    ordreT = IloIntArray(env, T) ;

    Init = IloBoolArray(env, n);
    L = IloIntArray(env, n);
    l = IloIntArray(env, n);
    P = IloNumArray(env, n);
    Pmax = IloNumArray(env, n);
    cf = IloNumArray(env, n);
    c0 = IloNumArray(env, n);
    cp = IloNumArray(env, n);
    nk = IloIntArray(env, n);
    firstOfSite = IloIntArray(env, n);
    D = IloNumArray(env, T);

    First = IloIntArray(env,n) ;
    Last = IloIntArray(env,n) ;


    //Lecture des données
    //Init
    while(nom!="["){
        fichier >> nom;
    }
    nom = "";
    for(IloInt j=0; j<n; j++){
        fichier >> Init[j];
    }

    //L
    while(nom!="["){
        fichier >> nom;
    }
    nom = "";
    for(IloInt j=0; j<n; j++){
        fichier >> L[j];
    }

    //l
    while(nom!="["){
        fichier >> nom;
    }
    nom = "";
    for(IloInt j=0; j<n; j++){
        fichier >> l[j];
    }

    //P
    while(nom!="["){
        fichier >> nom;
    }
    nom = "";
    for(IloInt j=0; j<n; j++){
        fichier >> P[j];
    }

    //Pmax
    while(nom!="["){
        fichier >> nom;
    }
    nom = "";
    for(IloInt j=0; j<n; j++){
        fichier >> Pmax[j];
    }

    //cf
    while(nom!="["){
        fichier >> nom;
    }
    nom = "";
    for(IloInt j=0; j<n; j++){
        fichier >> cf[j];
    }

    //c0
    while(nom!="["){
        fichier >> nom;
    }
    nom = "";
    for(IloInt j=0; j<n; j++){
        fichier >> c0[j];
    }

    //cp
    while(nom!="["){
        fichier >> nom;
    }
    nom = "";
    for(IloInt j=0; j<n; j++){
        fichier >> cp[j];
    }

    //D
    while(nom!="["){
        fichier >> nom;
    }
    nom = "";
    for(IloInt j=0; j<T; j++){
        fichier >> D[j];
    }



    //Symétries
    while(nom!="="){
        fichier >> nom;
    }
    nom = "";
    fichier >> K;

    while(nom!="["){
        fichier >> nom;
    }
    nom = "";
    for(IloInt j=0; j<n; j++){
        fichier >> nk[j];
    }

    //First
    while(nom!="["){
        fichier >> nom;
    }
    nom = "";
    for(IloInt j=0; j<n; j++){
        fichier >> First[j];
    }

    //Last
    while(nom!="["){
        fichier >> nom;
    }
    nom = "";
    for(IloInt j=0; j<n; j++){
        fichier >> Last[j];
    }



    //intra-site
    while(nom!="="){
        fichier >> nom;
    }
    nom = "";
    fichier >> S;

    while(nom!="["){
        fichier >> nom;
    }
    nom = "";
    for(IloInt j=0; j<n; j++){
        fichier >> firstOfSite[j];
    }

    cout <<"Sites: "<< firstOfSite << endl ;
    //fin lecture de fichier
}


double moyenne(IloNumArray V) {
    IloInt n = V.getSize() ;
    double moyP = 0 ;
    for (IloInt j =0 ; j < n ; j++) {
        moyP += V[j];
    }
    moyP = moyP/n ;
    return moyP ;
}

double variance(IloNumArray X, IloNumArray Y) {
    IloInt n = X.getSize() ;
    double sigma = 0 ;
    double moyX = moyenne(X);
    double moyY = moyenne(Y) ;
    for (IloInt j = 0 ; j < n ; j++) {
        sigma += (X[j] - moyX)*(Y[j] - moyY);
    }
    sigma = sigma/n ;

    return sigma ;
}


// Accès aux données

IloInt InstanceUCP::getn() const {
    return n ;
}

IloInt InstanceUCP::getT() const {
    return T ;
}

IloInt InstanceUCP::getS() const {
    return S ;
}

IloInt InstanceUCP::firstUnit(IloInt s) const {
    return firstUnitofSite[s] ;
}


IloInt InstanceUCP::getSiteOf(IloInt i) const {
    return SiteOf[i] ;
}

IloInt InstanceUCP::nbUnits(IloInt s) const {

    int nb= 1;
    int i=firstUnit(s)+1 ;
    while ((i < n) && firstOfSite[i]==0) {
        nb++ ;
        i++ ;
    }

return nb ;
}



IloBool InstanceUCP::getInit(IloInt i) const {
    if ((i >= n)||(i < 0)) {
        cout << "Attention, Init[" << i << "] n'existe pas." << endl ;
    }
    return Init[i] ;
}

IloInt InstanceUCP::getL(IloInt i) const {
    if ((i >= n)||(i < 0)) {
        cout << "Attention, L[" << i << "] n'existe pas." << endl ;
    }
    return L[i] ;
}

IloInt InstanceUCP::getl(IloInt i) const {
    if ((i >= n)||(i < 0)) {
        cout << "Attention, l[" << i << "] n'existe pas." << endl ;
    }
    return l[i] ;
}

IloNum InstanceUCP::getD(IloInt i) const {
    if ((i >= T)||(i < 0)) {
        cout << "Attention, D[" << i << "] n'existe pas." << endl ;
    }
    return D[i] ;
}

IloNum InstanceUCP::getPmin(IloInt i) const {
    if ((i >= n)||(i < 0)) {
        cout << "Attention, Pmin[" << i << "] n'existe pas." << endl ;
    }
    return P[i] ;
}

IloNum InstanceUCP::getPmax(IloInt i) const {
    if ((i >= n)||(i < 0)) {
        cout << "Attention, Pmax[" << i << "] n'existe pas." << endl ;
    }
    return Pmax[i] ;
}

IloNum InstanceUCP::getcf(IloInt i) const {
    if ((i >= n)||(i < 0)) {
        cout << "Attention, cf[" << i << "] n'existe pas." << endl ;
    }
    return cf[i] ;
}

IloNum InstanceUCP::getc0(IloInt i) const {
    if ((i >= n)||(i < 0)) {
        cout << "Attention, c0[" << i << "] n'existe pas." << endl ;
    }
    return c0[i] ;
}

IloNum InstanceUCP::getcp(IloInt i) const {
    if ((i >= n)||(i < 0)) {
        cout << "Attention, cp[" << i << "] n'existe pas." << endl ;
    }
    return cp[i] ;
}

IloNum InstanceUCP::getnbG() const {
    return nbG ;
}

IloNum InstanceUCP::getFirst(IloInt i)  const {
    if ((i >= n)||(i < 0)) {
        cout << "Attention, First[" << i << "] n'existe pas." << endl ;
    }
    return First[i] ;
}

IloNum InstanceUCP::getLast(IloInt i)  const {
    if ((i >= n)||(i < 0)) {
        cout << "Attention, Last[" << i << "] n'existe pas." << endl ;
    }
    return Last[i] ;
}
IloNum InstanceUCP::getGroup(IloInt i) const {
    if ((i >= n)||(i < 0)) {
        cout << "Attention, Group[" << i << "] n'existe pas." << endl ;
    }
    return Group[i] ;
}

IloNum InstanceUCP::getFirstG(IloInt i) const {
    if ((i >= n)||(i < 0)) {
        cout << "Attention, FirstG[" << i << "] n'existe pas." << endl ;
    }
    return FirstG[i] ;
}
IloNum InstanceUCP::getLastG(IloInt i) const {
    if ((i >= n)||(i < 0)) {
        cout << "Attention, LastG[" << i << "] n'existe pas." << endl ;
    }
    return LastG[i] ;
}

IloNum InstanceUCP::getSizeG(IloInt g) const {
    if ((g >= nbG)||(g < 0)) {
        cout << "Attention, SizeG[" << g << "] n'existe pas." << endl ;
    }
    return SizeG[g] ;
}

IloNum InstanceUCP::getordreT(IloInt t) const {
    if ((t >= T)||(t < 0)) {
        cout << "Attention, ordreT[" << t << "] n'existe pas." << endl ;
    }
    return ordreT[t] ;
}

IloInt InstanceUCP::getSommePmax()  const {
    return SommePmax ;
}

IloEnv InstanceUCP::getenv()  {
    return env;
}

IloInt InstanceUCP::getTri(IloInt i)  const {
    return Tri[i] ;
}


// Tri (Quicksort)

void InstanceUCP::quickSort(IloNumArray const & ordre, IloIntArray & indices, IloInt p, IloInt q) {
    IloInt r;
    if(p<q)
    {
        r=partition(ordre, indices, p,q);
        quickSort(ordre, indices,p,r);
        quickSort(ordre, indices,r+1,q);
    }
}

IloInt InstanceUCP::partition(IloNumArray const & ordre, IloIntArray & indices, IloInt p, IloInt q)
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
