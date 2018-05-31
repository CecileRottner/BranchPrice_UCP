#include "CplexPricingAlgo.h"
#include <iostream>

using namespace std;

DualCostsTime::DualCostsTime(InstanceUCP* inst) {
    int n = inst->getn() ;
    int T= inst->getT() ;
    Mu.resize(n*T, 0) ;
    Nu.resize(n*T, 0) ;
    Xi.resize(n*T, 0) ;
    Sigma.resize(T, 0) ;
}



CplexPricingAlgoTime::CplexPricingAlgoTime(InstanceUCP* inst, int t) {
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
    cplex.setParam(IloCplex::EpGap, 0.00001) ;

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
            obj.setLinearCoef(x[i],BaseObjCoefX.at(i)  + dual_coef );
            obj.setLinearCoef(p[i], inst->getcp(i)) ;
        }
        else {
            obj.setLinearCoef(x[i], dual_coef );
            obj.setLinearCoef(p[i], 0.0) ;
        }

    }
}


bool CplexPricingAlgoTime::findUpDownPlan(InstanceUCP* inst, const DualCostsTime & Dual, IloNumArray UpDownPlan, double& objvalue, double& realCost) {
    //returns True if an improving Up/Down plan has been found

    ofstream LogFile("LogFile.txt");
    cplex.setOut(LogFile);

    if ( !cplex.solve() ) {
        env.error() << "Failed to optimize Pricer with Cplex" << endl;
        exit(1);
    }

    if (cplex.getStatus()==CPX_STAT_INFEASIBLE){
        cout<<"NO SOLUTION TO PRICER"<<endl;
        cout<<endl<<" ************************* END PRICER with CPLEX"<<endl<<endl;

        return false;
    }
    else {

        int n = inst->getn() ;
        cplex.getValues(x, UpDownPlan) ;

        IloNumArray prod(env, n) ;
        cplex.getValues(p, prod) ;

        realCost = 0 ;
        for (int i=0 ; i <n ; i++) {
            realCost += UpDownPlan[i]*inst->getcf(i) + inst->getcp(i)*( inst->getPmin(i)*UpDownPlan[i] + prod[i]) ;
        }

       /* cout << "for time " << time << "; " << endl ;
        cout << "obj value without sigma: " << cplex.getObjValue() << endl;*/
        objvalue = cplex.getObjValue() - Dual.Sigma[time] ;
    }


    return true;
}
