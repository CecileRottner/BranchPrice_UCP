#include "CplexPricingAlgo.h"
#include <iostream>
#include <ctime>

using namespace std;


DynProgPricingAlgoTime::DynProgPricingAlgoTime(InstanceUCP* inst, const Parameters & par, int t) : Param(par) {
    //env=IloEnv() ;

    time=t;

    int n = inst->getn() ;

    W = inst->getSommePmax() - inst->getD(time) ;


    Table.resize((n+1)*(W+1), 0) ;

    //Initialisation des coefficients objectifs (primaux) de x
    BaseObjCoefX.resize(n, 0) ;
    totalBaseCost= 0 ;
    for (int i=0 ; i <n ; i++) {
        BaseObjCoefX.at(i) = inst->getcf(i) + (inst->getPmin(i))*inst->getcp(i) ;
        totalBaseCost += BaseObjCoefX.at(i) ;
    }

    ObjCoefX.resize(n, 0) ;
}

void DynProgPricingAlgoTime::updateObjCoefficients(InstanceUCP* inst, const Parameters & Param, const DualCostsTime & Dual, bool Farkas) {

    int n = inst->getn();
    int T = inst->getT() ;

    for (int i=0 ; i<n ; i++) {

        int L= inst->getL(i);
        int l= inst->getl(i);

        //// Calcul du cout réduit de x
        double dual_coef = 0 ;
        if (time>0) {
            dual_coef += - Dual.Mu.at(i*T + time) ;
        }
        if (time< T-1) {
            dual_coef += Dual.Mu.at(i*T + time+1) ;
        }
        if (time>=L) {
            dual_coef+= - Dual.Nu.at(i*T+ time) ;
        }

        if (time<=T-l-1) {
            dual_coef += - Dual.Xi.at(i*T + time + l) ;
        }

        /// Mise à jour des couts
        if (!Farkas) {
            ObjCoefX.at(i) = BaseObjCoefX.at(i)  + dual_coef ;
        }
        else {
            ObjCoefX.at(i) = dual_coef ;
        }

    }
}


bool DynProgPricingAlgoTime::findImprovingSolution(InstanceUCP* inst, const DualCostsTime & Dual, double& objvalue, double & temps_resolution, int exact) {
    //returns True if an improving Up/Down plan has been found

    int n = inst->getn();
    for (int i =1 ; i <=n ; i++) {
        for (int c=0 ; c <= W ; c++) {
            int pi = inst->getPmin(i-1) ;
            if (c >= pi) {
                Table.at(i*(W+1)+c) = fmax(Table.at((i-1)*(W+1)+c), Table.at((i-1)*(W+1)+c-pi) + ObjCoefX.at(i-1));
            }
            else {
                Table.at(i*(W+1)+c) = Table.at((i-1)*(W+1)+c);
            }
        }
    }

    double SumCosts = 0 ;
    for (int i=0 ; i <n ; i++) {
        SumCosts += ObjCoefX.at(i) ;
    }
    objvalue = SumCosts - Table.at(n*(W+1)+W) - Dual.Sigma[time] ;
    if (objvalue < - Param.Epsilon) {
        return true ;
    }
    return false ;

}

void DynProgPricingAlgoTime::getUpDownPlan(InstanceUCP* inst, const DualCostsTime & Dual, IloNumArray UpDownPlan, double& realCost, double & totalProd, bool Farkas) {

    int n = inst->getn();
    realCost=totalBaseCost;
    totalProd=inst->getSommePmax() ;

    for (int i=0 ; i <n ; i++) {
        UpDownPlan[i] = 1 ;
    }

    int c=W ;
    int i=n;
    while (c>0 && i>0) {

        if (  fabs( Table.at(i*(W+1)+c) - Table.at((i-1)*(W+1)+c) ) > Param.Epsilon ) {
            UpDownPlan[i-1] = 0 ;
            c= c - inst->getPmin(i-1) ;
            realCost -= BaseObjCoefX.at(i-1) ;
            totalProd -= inst->getPmin(i-1) ;
        }
        i-- ;
    }

}
