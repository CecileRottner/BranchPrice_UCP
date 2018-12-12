#include "CplexPricingAlgo.h"
#include <iostream>
#include <ctime>

using namespace std;


DynProgPricingAlgoTime::DynProgPricingAlgoTime(InstanceUCP* inst, Master_Model* M, const Parameters & par, int t) : Param(par) {
    //env=IloEnv() ;

    Master=M ;
    time=t;

    int n = inst->getn() ;

    W = inst->getSommePmax() - inst->getD(time) ;


    init.resize(n,-1) ;
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

        ObjCoefX.at(i) = 0 ;

        if (Param.doubleDecompo) {
            ObjCoefX.at(i) += Dual.Omega.at(i*T+time);
        }
        else {
            //// Couts primaux de x[i]
            if (!Farkas) {
                ObjCoefX.at(i) += BaseObjCoefX.at(i) ;
            }

            int L= inst->getL(i);
            int l= inst->getl(i);

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
                if (!inst->getLast(i)) {
                    if ( (time<=T-l-1) && (i < n-1) ) {
                        ObjCoefX.at(i) += - Dual.Epsilon.at(i*T + time + l) ;
                    }
                }
            }
        }
    }



    //// ajouts des couts duaux des interval up-set ////

    if (Master->nbIntUpSet>0) {
        list<IneqIntUpSet*>::const_iterator iup;

        //parcours des interval up set telles que t1=time
        for (iup = Master->IUP_t1[time].begin(); iup!= Master->IUP_t1[time].end() ; iup++) {
            int i = (*iup)->i ;
            ObjCoefX.at(i) -= (*iup)->dual ;
        }

        //parcours des interval up set telles que t0=time
        for (iup = Master->IUP_t0[time].begin(); iup!= Master->IUP_t0[time].end() ; iup++) {
            list<int>* C = (*iup)->C ;

            list<int>::const_iterator j;
            for (j=C->begin() ; j != C->end() ; j++) {
                ObjCoefX.at((*j)) -= (*iup)->dual ;
            }
        }
    }
}



bool DynProgPricingAlgoTime::findImprovingSolution(InstanceUCP* inst, const DualCostsTime & Dual, double& objvalue, double & temps_resolution, int exact) {
    //returns True if an improving Up/Down plan has been found

    int n = inst->getn();

    for (int c=0 ; c <= W ; c++) {
        Table.at(c) = 0 ;
    }
    for (int i =1 ; i <=n ; i++) {
        for (int c=0 ; c <= W ; c++) {
            int pi = inst->getPmin(i-1) ;
            if (c >= pi && (init.at(i-1) == -1) ) {
                Table.at(i*(W+1)+c) = fmax(Table.at((i-1)*(W+1)+c), Table.at((i-1)*(W+1)+c-pi) + ObjCoefX.at(i-1));
            }
            else {
                Table.at(i*(W+1)+c) = Table.at((i-1)*(W+1)+c);
            }
        }
    }

    double extraKPCost = 0 ;
    for (int i = 0 ; i <n ; i++) {
        if (init[i] == 0) {
            extraKPCost += ObjCoefX.at(i) ;
        }
    }

    double SumCosts = 0 ;
    for (int i=0 ; i <n ; i++) {
        SumCosts += ObjCoefX.at(i) ;
    }
    objvalue = SumCosts - Table.at(n*(W+1)+W) - extraKPCost - Dual.Sigma[time] ;
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


    //// il faut retirer i si init[i]==0
    for (int i = 0 ; i <n ; i++) {
        /// UpDownPlan[i] est forcément à 1
        if (init[i] == 0) {
            UpDownPlan[i] = 0 ;
            realCost -= BaseObjCoefX.at(i) ;
            totalProd -= inst->getPmin(i) ;
        }
    }

}
