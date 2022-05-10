#include "CplexPricingAlgo.h"
#include <iostream>
#include <ctime>

using namespace std;

DualCostsTime::DualCostsTime(InstanceUCP* inst) {
    int n = inst->getn() ;
    int T= inst->getT() ;
    Mu.resize(n*T, 0) ;
    Nu.resize(n*T, 0) ;
    Xi.resize(n*T, 0) ;
    Sigma.resize(T, 0) ;


    Epsilon.resize(n*T, 0) ; //RSU
    Delta.resize(n*T, 0) ; //RSD

    Omega.resize(n*T, 0) ; //equality constraint in double decomposition
}



CplexPricingAlgoTime::CplexPricingAlgoTime(InstanceUCP* inst, const Parameters & par, int t) : Param(par) {
    //env=IloEnv() ;

    time=t;

    int n = inst->getn() ;
    model = IloModel(env) ;


    x = IloBoolVarArray(env, n) ;

    p = IloNumVarArray(env, n, 0.0, 1000.0) ;

    obj = IloAdd(model, IloMinimize(env, 0.0));


    //Limite de production
    for (int i=0; i<n; i++) {
        model.add(p[i] <= (inst->getPmax(i)-inst->getPmin(i))*x[i] );
        model.add(p[i] >= 0);
    }

    //Demande
    IloExpr Prod(env) ;
    for (int i=0; i<n; i++) {
        Prod += p[i] + inst->getPmin(i)*x[i];
    }
    model.add(inst->getD(time) <= Prod);
    Prod.end() ;

    //Objectif pour p
    for (int i=0 ; i<n ; i++) {
        obj.setLinearCoef(p[i],inst->getcp(i));
    }

    cplex = IloCplex(model);
    cplex.setParam(IloCplex::EpGap, 0.01) ;

    //Initialisation des coefficients objectifs (primaux) de x
    BaseObjCoefX.resize(n, 0) ;
    for (int i=0 ; i <n ; i++) {
        BaseObjCoefX.at(i) = inst->getcf(i) + (inst->getPmin(i))*inst->getcp(i) ;
    }
}

void CplexPricingAlgoTime::updateObjCoefficients(InstanceUCP* inst, const Parameters & Param, const DualCostsTime & Dual, bool Farkas) {

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

        /// Mise à jour de la fonction objectif
        if (!Farkas) {
            if (Param.doubleDecompo) {
                obj.setLinearCoef(x[i], (1 - Param.costBalancing) * BaseObjCoefX.at(i)  + dual_coef );
            }
            else {
                obj.setLinearCoef(x[i],BaseObjCoefX.at(i)  + dual_coef );
            }
            obj.setLinearCoef(p[i], inst->getcp(i)) ;
        }
        else {
            obj.setLinearCoef(x[i], dual_coef );
            obj.setLinearCoef(p[i], 0.0) ;
        }

    }
}


bool CplexPricingAlgoTime::findImprovingSolution(InstanceUCP* inst, const DualCostsTime & Dual, double& objvalue, double & temps_resolution, int exact) {
    //returns True if an improving Up/Down plan has been found

    ofstream LogFile("LogFile.txt");
    cplex.setOut(LogFile);

    if (exact) {
        cplex.setParam(IloCplex::EpGap, Param.Epsilon) ;
    }
    else {
        cplex.setParam(IloCplex::EpGap, 0.1) ;
    }

    clock_t start;
    start = clock();

    if ( !cplex.solve() ) {
        env.error() << "Failed to optimize Pricer with Cplex" << endl;
        exit(1);
    }

    temps_resolution = ( clock() - start ) / (double) CLOCKS_PER_SEC;

    if (cplex.getStatus()==CPX_STAT_INFEASIBLE){
        cout<<"NO SOLUTION TO PRICER"<<endl;
        cout<<endl<<" ************************* END PRICER with CPLEX"<<endl<<endl;

        return false;
    }
    else {
        objvalue = cplex.getObjValue() - Dual.Sigma[time] ;
        if (objvalue < - Param.Epsilon) {
            return true ;
        }
    }
    return false;
}

void CplexPricingAlgoTime::getUpDownPlan(InstanceUCP* inst, const DualCostsTime & Dual, IloNumArray UpDownPlan, double& realCost, double & totalProd, bool Farkas) {


    int n = inst->getn();
    int T = inst->getT() ;

    cplex.getValues(x, UpDownPlan) ;

    if ( Farkas || !Param.DontGetPValue ) {
        IloNumArray prod(env, n) ;
        cplex.getValues(p, prod) ;
        realCost = 0 ;
        totalProd=0 ;
        for (int i=0 ; i <n ; i++) {
            if (UpDownPlan[i] > 1 - Param.Epsilon) {
                totalProd += inst->getPmin(i) + prod[i] ;
                realCost += inst->getcf(i) + inst->getcp(i)*( inst->getPmin(i) + prod[i]) ;
            }
        }
        //cout << "Total prod: " << totalProd << endl ;



    }

    else {
        realCost = cplex.getObjValue() ;

        for (int i=0 ; i<n ; i++) {

            int L= inst->getL(i);
            int l= inst->getl(i);

            if (UpDownPlan[i] > 1 - Param.Epsilon) {
                //// Cout réduit de x
                if (time>0) {
                    realCost -= - Dual.Mu.at(i*T + time) ;
                }
                if (time< T-1) {
                    realCost -= Dual.Mu.at(i*T + time+1) ;
                }
                if (time>=L) {
                    realCost -= - Dual.Nu.at(i*T+ time) ;
                }

                if (time<=T-l-1) {
                    realCost -= - Dual.Xi.at(i*T + time + l) ;
                }
            }
        }
    }




}
