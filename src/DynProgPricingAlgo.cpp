#include "CplexPricingAlgo.h"
#include <iostream>
#include <ctime>

using namespace std;


DynProgPricingAlgo::DynProgPricingAlgo(InstanceUCP* inst, Master_Model* M, const Parameters & par, int s) : Param(par) {
    //env=IloEnv() ;

    Master=M ;
    Site = s ;


    int nbS = Param.nbUnits(Site);

    int first = Param.firstUnit(Site) ;
    int T = inst->getT() ;


    Bellman.resize(2*T, 0) ;
    Prec.resize(2*T, 0) ;

    //Initialisation des coefficients objectifs (primaux) de x
    BaseObjCoefX.resize(nbS, 0) ;
    for (int i=0 ; i <nbS ; i++) {
        BaseObjCoefX.at(i) = inst->getcf(first+i) + (inst->getPmin(first+i))*inst->getcp(first+i) ;
    }

    ObjCoefX.resize(nbS*T, 0) ;
    ObjCoefU.resize(nbS*T, 0) ;

}

void DynProgPricingAlgo::updateObjCoefficients(InstanceUCP* inst, const Parameters & Param, const DualCosts & Dual, bool Farkas) {

    int T = inst->getT() ;
    int nbS = Param.nbUnits(Site);

    cout << "nbS = " << nbS << endl ;
    int first = Param.firstUnit(Site) ;

    for (int i=0 ; i<nbS ; i++) {
        for (int t=0 ; t < T ; t++) {

            ObjCoefX.at(i*T+t) = 0 ;
            if (!Farkas) {
                ObjCoefX.at(i*T + t) += BaseObjCoefX.at(i) ;
            }
            ObjCoefX.at(i*T + t) +=  - inst->getPmin(first+i)*Dual.Mu[t] - (inst->getPmax(first+i) - inst->getPmin(first+i))*Dual.Nu[(first+i)*T+t] ;
        }
    }

    for (int i=0 ; i<nbS ; i++) {
        for (int t=1 ; t < T ; t++) {

            ObjCoefU.at(i*T+t) = 0 ;
            if (!Farkas) {
                ObjCoefU.at(i*T + t) += inst->getc0(first) ;
            }
            ObjCoefU.at(i*T + t) +=  - inst->getPmin(first+i)*Dual.Mu[t] - (inst->getPmax(first+i) - inst->getPmin(first+i))*Dual.Nu[(first+i)*T+t] ;
        }
    }

    cout << "c0: " << inst->getc0(first) << endl ;

}



bool DynProgPricingAlgo::findImprovingSolution(InstanceUCP* inst, const DualCosts & Dual, double& objvalue) {

    //returns True if an improving Up/Down plan has been found

    int T = inst->getT() ;

    int i = Param.firstUnit(Site) ;

    int l = inst->getl(i);
    int L = inst->getL(i);
    double c0 = inst->getc0(i) ;
    cout << "c0 utilisé: " << c0 << endl ;
    cout << "cf: " <<  BaseObjCoefX.at(0);

    //initialisation

    Bellman.at(0*T+ 0) = 0 ;
    Bellman.at(1*T+ 0) = ObjCoefX.at(0) ;

    Prec.at(0*T+ 0) = -1 ;
    Prec.at(1*T+ 0) = -1;


    for (int t = 1 ; t < T ; t++) {

        ///// mise à jour de V(t, up) /////
        if ( t <= L-1 ) {
            Bellman.at(1*T+t) = Bellman.at(1*T+t-1) + ObjCoefX.at(t);
            Prec.at(1*T+t) = 1*T+t-1;
        }



        else if (t==T-1) {
            double up_prec = Bellman.at(1*T+t-1) + ObjCoefX.at(t);

            int from = t-1 ;


            double somme_obj = ObjCoefX.at(t) + c0;
            double down_prec = Bellman.at(0*T+t-1) + somme_obj;


            double best_down_prec = down_prec ;

            for (int k=t-2 ; k >= t - L ; k--) {
                somme_obj += ObjCoefX.at(k+1) ;
                down_prec = Bellman.at(0*T+k) + somme_obj ;
                if (down_prec < best_down_prec) {
                    best_down_prec =down_prec;
                    from=k;
                }
            }
            if (up_prec < best_down_prec) {
                Bellman.at(1*T+t) = up_prec ;
                Prec.at(1*T+t) = 1*T+t-1;
            }
            else {
                Bellman.at(1*T+t) =best_down_prec ;
                Prec.at(1*T+t) = 0*T+from;
            }

        }

        else {
            double up_prec = Bellman.at(1*T+t-1) + ObjCoefX.at(t);
            double down_prec = Bellman.at(0*T+t-L) + c0 + ObjCoefX.at(t);
            for (int k=t-L+1 ; k < t ; k++) {
                down_prec += ObjCoefX.at(k);
            }
            if (up_prec < down_prec) {
                Bellman.at(1*T+t) = up_prec ;
                Prec.at(1*T+t) = 1*T+t-1;
            }
            else {
                Bellman.at(1*T+t) = down_prec ;
                Prec.at(1*T+t) = 0*T+t-L;
            }
        }

        ///// mise à jour de V(t, down) //////
        if ( t <= l-1 ) {
            Bellman.at(0*T+t) = Bellman.at(0*T+t-1)  ;
            Prec.at(0*T+t) = 0*T+t-1;
        }


        //Si t==T-1, il y a plus de prédécesseurs (pas de min-down à satisfaire)
        else if (t==T-1) {
            double down_prec = Bellman.at(0*T+t-1) ;

            int from = t-1 ;
            double up_prec = Bellman.at(1*T+t-1) ;

            double best_up_prec = up_prec ;

            for (int k=t-2 ; k >= t - l ; k--) {
                up_prec = Bellman.at(1*T+k);
                //cout << "for k = " << k << ", up_prec: " << Bellman.at(1*T+k) << endl ;
                if (up_prec < best_up_prec) {
                    best_up_prec =up_prec;
                    from=k;
                }
            }

//            cout << "best up: " << best_up_prec << endl ;
//            cout << "down prec: " << down_prec << endl ;
            if (best_up_prec < down_prec) {
                cout << "ici" << endl ;
                Bellman.at(0*T+t) = best_up_prec ;
                Prec.at(0*T+t) = 1*T+from;
            }
            else {
                Bellman.at(0*T+t) = down_prec ;
                Prec.at(0*T+t) = 0*T+t-1;
            }

            double V_up_1 = Bellman.at(1*T+T-1) ;
            double V_down_2 = Bellman.at(0*T+T-1) ;
            //cout << "Bellman t=T-1 : " << V_up_1 << " " << V_down_2 << endl ;

        }
        else {
            double up_prec = Bellman.at(1*T+t-l)  ;
            double down_prec = Bellman.at(0*T+t-1) ;
            if (up_prec < down_prec) {
                Bellman.at(0*T+t) = up_prec ;
                Prec.at(0*T+t) = 1*T+t-l;
            }
            else {
                Bellman.at(0*T+t) = down_prec ;
                Prec.at(0*T+t) = 0*T+t-1;
            }
        }
    }

    double V_up = Bellman.at(1*T+T-1) ;
    double V_down = Bellman.at(0*T+T-1) ;

    cout << "Bellman: " ;
    for (int i=0 ; i <= 1 ; i++) {
        for (int t=0 ; t  < T ; t++) {
            cout << Bellman.at((!i)*T +t) << " " ;
        }
        cout << endl;
    }

    cout << "valeur sans sigma: " <<fmin(V_up, V_down) << endl ;

    objvalue = fmin(V_up, V_down) - Dual.Sigma[Site] ;

    if (objvalue < - Param.Epsilon) {
        return true ;
    }
    return false ;

}

void DynProgPricingAlgo::getUpDownPlan(InstanceUCP* inst, IloNumArray UpDownPlan) {

    int T = inst->getT() ;


    for (int t=0 ; t < T ; t++) {
        UpDownPlan[t] = 1 ;
    }


    double V_up = Bellman.at(1*T+T-1) ;
    double V_down = Bellman.at(0*T+T-1) ;

//    cout << "V_up : " << V_up << endl ;
//    cout << "V_down : " << V_down << endl ;

    int current = 0;
    int current_time = T-1 ;
    int status = 0 ;

    if (V_up < V_down) {
        current = 1*T+T-1;
        UpDownPlan[T-1]=1 ;
        status = 1;
    }
    else {
        current = 0*T+T-1;
        UpDownPlan[T-1]=0 ;
        status = 0;
    }

    while (current_time > 0) {
        int prec = Prec.at(current) ;
        int new_time = prec % T ;
        //cout << "prec: " << prec << endl;
        int new_status = prec / T ; // division entière

        UpDownPlan[new_time] = new_status ;

        for (int k= new_time + 1;  k < current_time ; k++ ) {
            UpDownPlan[k] = status ;
        }
        current = prec ;
        status=new_status ;
        current_time = new_time;
    }


    if (current_time == 0) {
        UpDownPlan[0] = status;
    }

    ///CHECK

    int first = Param.firstUnit(Site) ;
    double  cost = 0 ;
    for (int t = 0 ; t < T ; t++) {
        if (UpDownPlan[t]) {
            cost += ObjCoefX.at(t) ;

            if (t> 0 && !UpDownPlan[t-1] ) {
                cost += inst->getc0(first)  ;
            }
        }
    }

    cout << "check cost: " << cost << endl ;

}
