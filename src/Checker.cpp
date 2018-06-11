#include "Checker.h"
#include <iostream>

using namespace std;

CplexChecker::CplexChecker(InstanceUCP* instance, const Parameters & param) : Param(param) {

    inst = instance;
    model = IloModel(env) ;

    int T = inst->getT() ;
    int n = inst->getn();

    x = IloBoolVarArray(env, n*T) ;
    u = IloBoolVarArray(env, n*T) ;

    pp = IloNumVarArray(env, n*T, 0.0, 1000);


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

    if (Param.Ramp) {
        cout << "gradients" << endl;
        for (int i = 0 ; i <n ; i++) {
           // model.add(pp[i*T] <= 0 ) ;
            for (int t = 1 ; t < T ; t++) {
                model.add(pp[i*T + t] - pp[i*T + t-1] <= (inst->getPmax(i)-inst->getPmin(i))*x[i*T + t-1]/3 );
                model.add(pp[i*T + t-1] - pp[i*T + t] <= (inst->getPmax(i)-inst->getPmin(i))*x[i*T + t]/2 );
            }
        }
    }


    //Contraintes intra-site
    if (Param.IntraSite) {
        int S = inst->getS() ;
        for (int s = 0 ; s < S ; s++) {
            int first = inst->firstUnit(s) ;
            for (int t=1 ; t < T ; t++) {
                IloExpr sum2(env) ;
                for (int i=0 ; i < inst->nbUnits(s) ; i++) {
                    sum2+=u[(first+i)*T+t] ;
                }
                model.add(sum2 <= 1);
                sum2.end() ;
            }
        }
    }


}

double CplexChecker::getIntegerObjValue() {

    int print=0 ;
    IloModel IntegerModel(env) ;
    IntegerModel.add(model) ;

    IloCplex IntegerObjCplex = IloCplex(IntegerModel) ; // ou juste valeur opt entière
    IntegerObjCplex.setParam(IloCplex::EpGap, 0) ;
    IntegerObjCplex.solve() ;

    IntegerObj = IntegerObjCplex.getBestObjValue() ;

    int n = inst->getn();
    int T= inst->getT() ;
    if (print) {

        IloNumArray solution = IloNumArray(env, n*T) ;
        IntegerObjCplex.getValues(solution, x) ;

        cout.precision(6);
        cout << "X: " << endl ;
        for (int t=0 ; t < T ; t++) {
            for (int i=0 ; i < n ; i++) {
                cout << fabs(solution[i*T+t]) << " " ;
            }
            cout << endl ;
        }
        cout << endl ;
    }

    return IntegerObj;
}

double CplexChecker::getLRValue() {

    //Modèle
    IloModel LRModel(env) ;
    LRModel.add(model) ;
    LRModel.add(IloConversion(env, x, IloNumVar::Float) ) ;
    LRModel.add(IloConversion(env, u, IloNumVar::Float) ) ;

    //Résolution
    IloCplex LRVal = IloCplex(LRModel) ;
    LRVal.setParam(IloCplex::EpGap, 0) ;
    LRVal.solve() ;

    LRValue = LRVal.getObjValue();
    return LRValue;
}

double CplexChecker::getLRCplex() {

    //Modèle
    IloModel LRCplexModel(env) ;
    LRCplexModel.add(model) ;

    //Résolution
    IloCplex LRCplex = IloCplex(LRCplexModel) ;
    LRCplex.setParam(IloCplex::EpGap, 0) ;
    LRCplex.setParam(IloCplex::Param::MIP::Limits::Nodes, 1) ;
    LRCplex.solve() ;

    LRCplexVal = LRCplex.getBestObjValue() ;
    return LRCplexVal ;
}


void CplexChecker::CplexPrimalHeuristic(IloNumArray solution, IloNumArray solution_p) {

    //Modèle
    IloModel LRCplexModel(env) ;
    LRCplexModel.add(model) ;

    //Résolution
    IloCplex LRCplex = IloCplex(LRCplexModel) ;
    LRCplex.setParam(IloCplex::EpGap, 0) ;
    LRCplex.setParam(IloCplex::Param::MIP::Limits::Nodes, 1) ;
    LRCplex.solve() ;

    LRCplex.getValues(solution, x) ;
    LRCplex.getValues(solution_p, pp) ;
}

//double CplexChecker::printSolution() {
//    int T = inst->getT() ;
//    int n = inst->getn();
//  /*  cplex.solve() ;
//    double objvalue = cplex.getObjValue() ;*/
//    IloNumArray solution = IloNumArray(env, n*T) ;
//    IloNumArray solution_u = IloNumArray(env, n*T) ;
//    IloNumArray solution_p = IloNumArray(env, n*T) ;
//    cplex.getValues(solution, x) ;
//    cplex.getValues(solution_u, u) ;
//    cplex.getValues(solution_p, pp) ;

//    cout.precision(6);
//    cout << "X: " << endl ;
//    for (int t=0 ; t < T ; t++) {
//        for (int i=0 ; i < n ; i++) {
//            cout << fabs(solution[i*T+t]) << " " ;
//        }
//        cout << endl ;
//    }
//    cout << endl ;

//    cout << "U: " << endl ;
//    for (int t=0 ; t < T ; t++) {
//        for (int i=0 ; i < n ; i++) {
//            cout << fabs(solution_u[i*T+t]) << " " ;
//        }
//        cout << endl ;
//    }
//    cout << endl ;



//    return 0 ;
//}



void CplexChecker::checkSolution(const vector<double> & x_frac) {

    double eps = 0.000001 ;

    cout << "start check..." << endl;

    int T = inst->getT() ;
    int n = inst->getn();

    IloModel CheckModel(env) ;

    IloNumVarArray p = IloNumVarArray(env, n*T, 0.0, 1000);

    vector<double> u_frac(n*T, 0) ;
    for (int i=0 ; i <n ; i++) {
        for (int t=1 ; t < T ; t++) {
            if ( x_frac[i*T+t-1] < x_frac[i*T+t]  ) {
                u_frac[i*T+t] = x_frac[i*T+t] - x_frac[i*T+t-1] ;
            }
        }
    }


    // Objective Function: Minimize Cost
    IloExpr cost(env) ;
    for (int t=0 ; t < T ; t++) {
        for (int i=0; i<n; i++) {
            cost +=  x_frac[i*T + t]*inst->getcf(i) + inst->getc0(i)*u_frac[i*T + t] + (p[i*T + t]+inst->getPmin(i)*x_frac[i*T + t])*(inst->getcp(i)) ;
        }
    }

    CheckModel.add(IloMinimize(env, cost));

    //Limite de production
    for (int i=0; i<n; i++) {
        for (int t=0 ; t < T ; t++) {
            CheckModel.add(p[i*T + t] <= (inst->getPmax(i)-inst->getPmin(i))*x_frac[i*T + t]);
            CheckModel.add(p[i*T + t] >= 0);
        }
    }

    //Demande
    for (int t=0; t < T ; t++) {
        IloExpr Prod(env) ;
        for (int i=0; i<n; i++) {
            Prod += p[i*T + t] + inst->getPmin(i)*x_frac[i*T + t];
        }
        CheckModel.add(inst->getD(t) <= Prod);
        Prod.end() ;
    }

    if (Param.Ramp) {
        cout << "gradients" << endl;
        for (int i = 0 ; i <n ; i++) {
           // model.add(pp[i*T] <= 0 ) ;
            for (int t = 1 ; t < T ; t++) {
                CheckModel.add(p[i*T + t] - p[i*T + t-1] <= (inst->getPmax(i)-inst->getPmin(i))*x[i*T + t-1]/3 );
                CheckModel.add(p[i*T + t-1] - p[i*T + t] <= (inst->getPmax(i)-inst->getPmin(i))*x[i*T + t]/2 );
            }
        }
    }

    // Min up constraints
    for (int i=0; i<n; i++) {
        for (int t=inst->getL(i) ; t < T ; t++) {
            double sum = 0 ;
            for (int k= t - inst->getL(i) + 1; k <= t ; k++) {
                sum += u_frac[i*T + k] ;
            }
            if (sum > x_frac[i*T + t] + eps) {
                cout << "min up " << i << ", " << t << " non satisfaite" << endl ;
            }
        }
    }

    // Min down constraints
    for (int i=0; i<n; i++) {
        for (int t=inst->getl(i) ; t < T ; t++) {
            double sum=0 ;
            for (int k= t - inst->getl(i) + 1; k <= t ; k++) {
                sum += u_frac[i*T + k] ;
            }
            if (sum > 1 - x_frac[i*T + t - inst->getl(i)] + eps) {
                cout << "min down " << i << ", " << t << " non satisfaite" << endl ;

                cout << "sum: " << sum << endl ;
                cout << "RHS: " << 1 - x_frac[i*T + t - inst->getl(i)]  << endl ;
            }
        }
    }


    //Contraintes intra-site
    if (Param.IntraSite) {
    int S = inst->getS() ;
    for (int s = 0 ; s < S ; s++) {
        int first = inst->firstUnit(s) ;
        for (int t=1 ; t < T ; t++) {
            double sum=0 ;
            for (int i=0 ; i < inst->nbUnits(s) ; i++) {
                sum+=u_frac[(first+i)*T+t] ;
            }
            if (sum > 1+eps) {
                cout << "intrasite " << s << ", "<< t << " non satisfaite" << endl;
            }
        }
    }

    }





    IloCplex CheckCplex = IloCplex(CheckModel) ;

    cout << "Solve..." << endl ;
    CheckCplex.solve() ;
    cout << "end solve" << endl ;

    int fea = CheckCplex.isPrimalFeasible();
    cout << "feasible: " << fea << endl ;
    double value = CheckCplex.getObjValue() ;
    cout << "value: " <<  value << endl ;

    //    IloNumArray solution_p = IloNumArray(env, n*T) ;
    //    CheckCplex.getValues(solution_p, p) ;
    //    cout << solution_p << endl ;


}

