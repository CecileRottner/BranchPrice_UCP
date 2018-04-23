#include "Checker.h"
#include <iostream>

using namespace std;

CplexChecker::CplexChecker(InstanceUCP* instance) {

    inst = instance;
    model = IloModel(env) ;

    int T = inst->getT() ;
    int n = inst->getn();

    x = IloBoolVarArray(env, n*T) ;
    u = IloBoolVarArray(env, n*T) ;

    IloNumVarArray pp(env, n*T, 0.0, 1000);


    // Objective Function: Minimize Cost
    IloExpr cost(env) ;
    for (int t=0 ; t < T ; t++) {
        for (int i=0; i<n; i++) {
            cost += x[i*T + t]*inst->getcf(i) + inst->getc0(i)*u[i*T + t] + (pp[i*T + t]+inst->getPmin(i)*x[i*T + t])*(inst->getcp(i)) ;
        }
    }

    model.add(IloMinimize(env, cost));

    // Min up constraints
    for (int i=0; i<n; i++) {
        for (int t=inst->getL(i) ; t < T ; t++) {
            IloExpr sum(env) ;
            for (int k= t - inst->getL(i) + 1; k <= t ; k++) {
                sum += u[i*T + k] ;
            }
            model.add(sum <= x[i*T + t]) ;
            sum.end() ;
        }
    }

    // Min down constraints
    for (int i=0; i<n; i++) {
        for (int t=inst->getl(i) ; t < T ; t++) {
            IloExpr sum(env) ;
            for (int k= t - inst->getl(i) + 1; k <= t ; k++) {
                sum += u[i*T + k] ;
            }
            model.add(sum <= 1 - x[i*T + t - inst->getl(i)]) ;
            sum.end() ;
        }
    }

    //Relation entre u et x
    for (int i=0; i<n; i++) {
        for (int t=1 ; t < T ; t++) {
            model.add(x[i*T + t] - x[i*T + t-1] <= u[i*T + t]);
        }
    }

    //Contraintes intra-site
    for (int t=1 ; t < T ; t++) {
        IloExpr sum(env) ;
        for (int i=0 ; i <n ; i++) {
            sum+=u[i*T+t] ;
        }
        model.add(sum <= 1);
        sum.end() ;
    }



    //Limite de production
    for (int i=0; i<n; i++) {
        for (int t=0 ; t < T ; t++) {
            model.add(pp[i*T + t] <= (inst->getPmax(i)-inst->getPmin(i))*x[i*T + t]);
            model.add(pp[i*T + t] >= 0);
        }
    }


    //Demande
    for (int t=0; t < T ; t++) {
        IloExpr Prod(env) ;
        for (int i=0; i<n; i++) {
            Prod += pp[i*T + t] + inst->getPmin(i)*x[i*T + t];
        }
        model.add(inst->getD(t) <= Prod);
        Prod.end() ;
    }

    int conti = 0;
    if (conti) {
        model.add(IloConversion(env, x, IloNumVar::Float) ) ;
        model.add(IloConversion(env, u, IloNumVar::Float) ) ;
    }

    cplex = IloCplex(model);
}

int CplexChecker::check() {
    int T = inst->getT() ;
    int n = inst->getn();
    cplex.solve() ;
    double objvalue = cplex.getObjValue() ;
    IloNumArray solution = IloNumArray(env, n*T) ;
    cplex.getValues(solution, x) ;

    for (int t=0 ; t < T ; t++) {
        for (int i=0 ; i < n ; i++) {
            cout << fabs(solution[i*T+t]) << " " ;
        }
        cout << endl ;
    }
    cout << endl ;
    return objvalue ;
}


