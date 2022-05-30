#include "CplexPricingAlgo.h"
#include <iostream>
#include <ctime>

using namespace std;


DynProgPricingAlgoTimePower::DynProgPricingAlgoTimePower(InstanceUCP* inst, Master_Model* M, const Parameters & par, int t) 
: DynProgPricingAlgoTime(inst, M, par, t){

    int n = inst->getn() ;

    init.resize(n,-1) ;
    power.resize(n,0) ;

    Dt = inst->getD(time);

    Table.resize(n);
    BaseObjCoefX.resize(n, 0) ;
    BaseObjCoefP.resize(n, 0) ;

    for (int i=0 ; i <n ; i++) {
        Table[i].resize(n*(Dt+1), 0) ;
        BaseObjCoefX.at(i) = (1 - Param.costBalancing.at(i)) * inst->getcf(i) ;
        if (par.PminOnLambda){
            BaseObjCoefX.at(i) -= Param.costBalancing.at(i) * inst->getcp(i) * inst->getPmin(i);
        }
        if (par.PmaxOnLambda){
            BaseObjCoefX.at(i) -= Param.costBalancing.at(i) * inst->getcp(i) * inst->getPmax(i);
        }
        BaseObjCoefP.at(i) = inst->getcp(i) ;
    }

    ObjCoefX.resize(n, 0) ;
    ObjCoefP.resize(n, 0) ;

    pivotUnit = -1 ;
}

void DynProgPricingAlgoTimePower::updateObjCoefficients(InstanceUCP* inst, const Parameters & Param, const DualCostsTime & Dual, bool Farkas){
    
    int n = inst->getn();
    int T = inst->getT() ;

    for (int i=0 ; i<n ; i++) {
        
        int L= inst->getL(i);
        int l= inst->getl(i);


        ObjCoefX.at(i) = 0 ;
        ObjCoefP.at(i) = 0 ;

        if (Param.doubleDecompo) {
            
            ObjCoefX.at(i) += Dual.Omega.at(i*T+time);

            if (Param.minUpDownDouble) {
                if (time>0) {
                    ObjCoefX.at(i) += - Dual.Mu.at(i*T + time) ;
                }
                if (time< T-1) {
                    ObjCoefX.at(i) += Dual.Mu.at(i*T + time+1) ;
                }
                if (time>=L) {
                    ObjCoefX.at(i) += - Dual.Nu.at(i*T+ time) ;
                }

                if (time<=T-l-1) {
                    ObjCoefX.at(i) += - Dual.Xi.at(i*T + time + l) ;
                }
            }

            if (!Farkas) {
                ObjCoefX.at(i) += BaseObjCoefX.at(i) ;
                ObjCoefP.at(i) += BaseObjCoefP.at(i) ;
            }

        }


        else {
            //// Couts primaux de x[i]
            if (!Farkas) {
                ObjCoefX.at(i) += BaseObjCoefX.at(i) ;
                ObjCoefP.at(i) += BaseObjCoefP.at(i) ;
            }


            //// Calcul du cout réduit de x
            if (time>0) {
                ObjCoefX.at(i) += - Dual.Mu.at(i*T + time) ;
            }
            if (time< T-1) {
                ObjCoefX.at(i) += Dual.Mu.at(i*T + time+1) ;
            }
            if (time>=L) {
                ObjCoefX.at(i) += - Dual.Nu.at(i*T+ time) ;
            }

            if (time<=T-l-1) {
                ObjCoefX.at(i) += - Dual.Xi.at(i*T + time + l) ;
            }

            /// couts duaux SSBI
            if (Param.masterSSBI) {
                //RSU
                if (!inst->getLast(i)) {
                    if ( (time<=T-l-1) && (i < n-1) ) {
                        ObjCoefX.at(i) += - Dual.Epsilon.at(i*T + time + l) ;
                    }
                }

                //RSD
                if (!Param.RSUonly) {
                if (!inst->getLast(i) && time >= L) {
                    ObjCoefX.at(i) += - Dual.Delta.at(i*T + time) ;
                }
                if (i>0 && !inst->getLast(i-1) && time >= L) {
                    ObjCoefX.at(i) += Dual.Delta.at((i-1)*T + time) ;
                }

                if (time<T-1 && !inst->getLast(i) && time+1>= L) {
                    ObjCoefX.at(i) += Dual.Delta.at(i*T + time+1) ;
                }
                }
            }
        }
    }
}

bool DynProgPricingAlgoTimePower::findImprovingSolution(InstanceUCP* inst, const DualCostsTime & Dual, double& objvalue, double & temps_resolution, int exact) {
    
    pivotUnit = -1 ;

    int n = inst->getn() ;

    for (int i = 0 ; i < n ; i++) {
        // init == 1 (unité forcément allumée) : cout non nul pour demande nulle
        if (init[i] == 1) {
            Table[i].at(0) = ObjCoefX.at(i) + inst->getPmin(i) * ObjCoefP.at(i) ;
        }


        // cas classique : 

        else {
            Table[i].at(0) = 0 ;
        }

        if (init[i] != 0){
            for (int c=1 ; c <= fmin(inst->getPmin(i),Dt)  ; c++) {
                Table[i].at(c) = ObjCoefX.at(i) + inst->getPmin(i) * ObjCoefP.at(i) ;
            }

            for (int c = inst->getPmin(i) + 1; c <= fmin(inst->getPmax(i),Dt)  ; c++) {
                Table[i].at(c) = ObjCoefX.at(i) + c * ObjCoefP.at(i) ;
            }
        }


        // init==0 (unité forcément éteinte) : impossible de satisfaire la demande
        else{
            for (int c=1 ; c <= fmin(inst->getPmax(i),Dt)  ; c++) {
                Table[i].at(c) = std::numeric_limits<float>::max() / 10;
            }
        }

        // au dessus de Pmax : impossible de satisfaire la demande
        for (int c = inst->getPmax(i) + 1 ; c <= Dt  ; c++) {
            Table[i].at(c) = std::numeric_limits<float>::max() / 10;
        }

        double off = 0;
        double on = 0;

        int avantI = 1;

        // on parcourt les autres unités pour remplir le tableau
        for (int j = 0 ; j < n ; j++) {
            if (j == i) {
                avantI = 0;
            }

            else {

                for (int c = 0 ; c <= Dt  ; c++) {

                    off = Table[i].at( (j + avantI - 1) * (Dt+1) + c);
                    on = fmin( Table[i].at( (j + avantI - 1) * (Dt+1) + fmax(0, c - inst->getPmin(j)) ) + ObjCoefX.at(j) + ObjCoefP.at(j) * inst->getPmin(j), 
                               Table[i].at( (j + avantI - 1) * (Dt+1) + fmax(0, c - inst->getPmax(j)) ) + ObjCoefX.at(j) + ObjCoefP.at(j) * inst->getPmax(j));
                    if (init[i]==0) {
                        Table[i].at( (j + avantI) * (Dt+1) + c) = off;
                    }
                    else if (init[i]==1) {
                        Table[i].at( (j + avantI) * (Dt+1) + c) = on;
                    }
                    else {
                        Table[i].at( (j + avantI) * (Dt+1) + c) = fmin(off, on);
                    }
                }

            }
        }

        if ( Table[i].at( (n-1) *(Dt+1) + Dt) - Dual.Sigma[time] < objvalue - Param.Epsilon ){
            objvalue = Table[i].at( (n-1) *(Dt+1) + Dt) - Dual.Sigma[time] ;
            pivotUnit = i;
            if (exact > 0.5){
                if (objvalue < - Param.heurPricingThreshold * fabs(Dual.Sigma[time])){
                    return true ;
                }
            }
        }

    }


    /*
    for (int i = 0 ; i < n ; i++) {
        cout << "i: " << i << endl;
        cout << endl;
        for (int j = 0 ; j < n ; j++) {
            cout << "j: " << j << endl;
            for (int c = 0 ; c <= Dt  ; c++) {
                cout << Table[i].at(j * (Dt+1) + c) << " " ;
            }
            cout << endl;
        }
        cout << endl;
    }
    */

    if (pivotUnit >= 0) {
        return true ;
    }
    return false ;
}   

void DynProgPricingAlgoTimePower::getUpDownPlan(InstanceUCP* inst, const DualCostsTime & Dual, IloNumArray UpDownPlan, IloNumArray PowerPlan, double& realCost, double & totalProd, bool Farkas) {

    int n = inst->getn();

    for (int i=0 ; i <n ; i++) {
        UpDownPlan[i] = 0 ;
        PowerPlan[i] = 0 ;
    }

    int c = Dt ;
    int i = n - 1;

    int avantI = 0;
    while (c > 0 && i >= 0) {

        if (i == pivotUnit){
            avantI = 1;
            i--;
        }

        else{
            if (  fabs( Table[pivotUnit].at( (i + avantI) *(Dt+1)+c) - Table[pivotUnit].at(( i + avantI -1) * (Dt+1)+c) ) > Param.Epsilon ) {

                UpDownPlan[i] = 1 ;

                if (  fabs( Table[pivotUnit].at( (i + avantI) * (Dt+1) + c ) 
                            - ( Table[pivotUnit].at( (i + avantI - 1)*(Dt+1) + fmax(0, c - inst->getPmin(i)) ) + ObjCoefX.at(i) + ObjCoefP.at(i) * inst->getPmin(i) ) ) 
                        > Param.Epsilon ) {
                    c = fmax(0, c - inst->getPmax(i)) ;
                    PowerPlan[i] = inst->getPmax(i) ;
                }

                else {
                    c = fmax(0, c - inst->getPmin(i)) ;
                    PowerPlan[i] = inst->getPmin(i) ;
                }
            }
            i-- ;
        }
        
    }

    if (c > 0) {
        UpDownPlan[pivotUnit] = 1 ;
        PowerPlan[pivotUnit] = fmax(c, inst->getPmin(pivotUnit)) ;
    }

}
