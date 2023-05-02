#include "CplexPricingAlgo.h"
#include <iostream>
#include <ctime>
#include <cmath>

using namespace std;

bool DynProgPricingAlgo::checkTransitionSUSD(InstanceUCP* inst,int prec_time, int current_time, int current_status) {

    if (current_time <= prec_time) {
        return false;
    }

    int i = Param.firstUnit(Site) ;
    int T = inst->getT() ;

    // on definit les indices t_up_min et t_up_max entre lesquels il faut vérifier que les décisions de branchement ne contredisent pas
    // la décision de laisser l'unité en marche sur [t_up_min,t_up_max]
    int t_up_min=T;
    int t_up_max=T-1;

    // pareil pour down
    int t_down_min=T;
    int t_down_max=T-1;

// calcul des t_up et t_down
    if (prec_time==-1) { // le noeud précédent est la source
        if (current_status==1) {
            t_up_min=0;
            t_up_max=current_time;
        }
        else {
            t_down_min=0 ;
            t_down_max=current_time;
        }
    }
    else if (current_time==T) { // le noeud d'arrivée est le puits
        if (current_status==1) {
            t_up_min=prec_time;
            t_up_max=T-1;
        }
        else {
            t_down_min=prec_time ;
            t_down_max=T-1;
        }
    }

    else if (current_status==0) { // dans ce cas on a une transition V(up, t0) à V(down, t1)
        if (prec_time>0) { // si prec_time = 0 on a le droit de faire une transition pour courte que L (on n'a pas démarré en 0)
            if (current_time - prec_time < inst->getL(i) ) {
                return false;
            }
        }
        t_up_min=prec_time;
        t_up_max=current_time-1;
        t_down_min=current_time;
        t_down_max= current_time;
    }

    else if (current_status==1) { // dans ce cas on a une transition V(down, t0) à V(up, t1)
        if (prec_time>0) { // si prec_time = 0 on a le droit de faire une transition pour courte que l (on ne s'est pas éteint en 0)
            if (current_time - prec_time < inst->getl(i) ) {
                return false;
            }
        }
        t_up_min=current_time;
        t_up_max=current_time;
        t_down_min=prec_time;
        t_down_max= current_time-1;
    }


    for (int t=t_up_min ; t <= t_up_max ; t++) {
        if ( branchingDecisions.at(t) != 8 and  branchingDecisions.at(t) !=  1 ) {
            return false;
        }
    }

    for (int t=t_down_min ; t <= t_down_max ; t++) {
        if ( branchingDecisions.at(t) != 8 and  branchingDecisions.at(t) != 0 ) {
            return false;
        }
    }

    return true ;
}

double DynProgPricingAlgo::computeStartUpCosts(InstanceUCP* inst,  const DualCosts & Dual, int prec_time, int current_time) {

    int i = Param.firstUnit(Site) ;
    int T = inst->getT() ;
    double cost = (Dual.ObjCoefU).at(i*T + current_time);
    double c0 = inst->getc0(i) ;
    if (Param.nonLinearStartUpCost) {
        cost -= c0 * exp(-float(current_time - prec_time)/T);
    }
    return cost ;
}

bool DynProgPricingAlgo::findImprovingSolutionSUSD(InstanceUCP* inst, const DualCosts & Dual, double& objvalue) {

    int T = inst->getT() ;

    // cout << "branching decisions: " ;
    // for (int t=0 ; t < T ; t++) {
    //     cout << branchingDecisions.at(t) << " " ;
    // }
    // cout << endl ;


    int i = Param.firstUnit(Site) ;

    int l = inst->getl(i);
    int L = inst->getL(i);
    cout << "l : " << l << endl ; 
    cout << "L : " << L << endl ;


    //initialisation
    for (int t=0; t < 2*T ; t++) {
        Bellman.at(t) = std::numeric_limits<double>::infinity(); ;
    }

    if ( checkTransitionSUSD( inst, -1, 0, 0) ) {
        Bellman.at(0*T+ 0) = 0 ;
    }
    if ( checkTransitionSUSD( inst, -1, 0, 1) ) {
        Bellman.at(1*T+ 0) = 0 ; // cf(t) sera pris en compte par le prochain noeud
    }

    Prec.at(0*T+ 0) = -1 ;
    Prec.at(1*T+ 0) = -1;


    for (int t = 1 ; t < T ; t++) {

        ///// Calcul de V(t, up) /////

        // cas où prec=source
        if (checkTransitionSUSD(inst,-1, t, 1)) {
            double bell=0;
            for (int k=0 ; k < t ; k++) {
                bell += (Dual.ObjCoefX).at(i*T + k);
            }
            //cout << "bell: " << bell << endl ;
            Bellman.at(1*T+t) = bell;
            Prec.at(1*T+t) = -1;
        }

        for (int k = 0 ; k < t ; k++) {

            if (checkTransitionSUSD(inst, k, t, 1)) {

                double bell=Bellman.at(0*T+k) + computeStartUpCosts(inst, Dual, k,t);
                if (bell < Bellman.at(1*T+t) ) {
                    Bellman.at(1*T+t) = bell;
                    Prec.at(1*T+t) = k;
                }
            }
        }
        
        ///// calcul de V(t, down) //////

        // arc de la source à V(t,down) vaut 0

        if (checkTransitionSUSD(inst,-1, t, 0)) {
            Bellman.at(0*T+t) = 0;
            Prec.at(0*T+t) = -1;
        }

        for (int k = 0 ; k < t ; k++) {

            if (checkTransitionSUSD(inst, k, t, 0)) {

                double bell=Bellman.at(1*T+k) ;
                for (int time = k ; time < t ; time++) {
                    bell += (Dual.ObjCoefX).at(i*T + time);
                }
                if (bell < Bellman.at(0*T+t) ) {
                    Bellman.at(0*T+t) = bell;
                    Prec.at(0*T+t) = k;
                }
            }
        }
    }


    double V_sink= 0 ;


    time_prec_sink=-1;
    status_prec_sink=0;



    for (int t=0 ; t  < T ; t++) {

        // transition V(up,t) --> sink
        if ( checkTransitionSUSD(inst, t, T, 1) ) {
            double bell = Bellman.at(1*T+t);
            for (int k=t; k <= T-1 ; k++) {
                bell += (Dual.ObjCoefX).at(i*T + k);
            }
            if (bell < V_sink ) {
                V_sink=bell;
                time_prec_sink=t;
                status_prec_sink=1;
            }
        }
        if ( checkTransitionSUSD(inst, t, T, 0) ) {
            double bell = Bellman.at(0*T+t);
            if (bell < V_sink ) {
                V_sink=bell;
                time_prec_sink=t;
                status_prec_sink=0;
            }
        }
    }



    int print=0;
    if (print) cout << "Bellman: " ;
    for (int i=0 ; i <= 1 ; i++) {
        for (int t=0 ; t  < T ; t++) {
           if (print) cout << Bellman.at((i)*T +t) << " " ;
       }
       if (print) cout << endl;
   }


//    cout << "valeur sans sigma: " <<fmin(V_up, V_down) << endl ;

   objvalue = V_sink - Dual.Sigma[Site] ;

    if (objvalue <= std::numeric_limits<double>::max() / 2) {
        return true ;
    }


    return false ;
}

void DynProgPricingAlgo::getUpDownPlanSUSD(InstanceUCP* inst, IloNumArray UpDownPlan) {

    int T = inst->getT() ;

    for (int t=fmax(time_prec_sink,0) ; t < T ; t++) {
        UpDownPlan[t] = status_prec_sink ;
    }

    int prec=time_prec_sink;
    int status = status_prec_sink;
    while (prec > 0) {

        int precprec= Prec.at(status*T+ prec) ;
        if (precprec >= 0) {
            for (int t=precprec ; t < prec ; t++) {
                UpDownPlan[t] = !status;
            }
        }
        else { // precprec = -1: dans ce cas, on reste au même statut jusqu'à prec
            for (int t=0 ; t < prec ; t++) {
                UpDownPlan[t] = status;
            }
        }
        UpDownPlan[prec]=status; // au cas ou

        prec=precprec;
        status=!status;
    }

}