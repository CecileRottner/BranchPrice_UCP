#include "Pricer.h"
#include "scip/cons_linear.h"
#include <map>
#include <vector>
#include <iostream>


#define OUTPUT_PRICER
// à décommenter pour l'affichage de debug

using namespace std;
using namespace scip;


/** Constructs the pricer object with the data needed
 *
 *  An alternative is to have a problem data class which allows to access the data.
 */
ObjPricerTimeUCP::ObjPricerTimeUCP(
        SCIP*                                scip,          /**< SCIP pointer */
        const char*                         pp_name,      /**< name of pricer */
        MasterTime_Model*                        M,
        InstanceUCP*                        instance,
        const Parameters &                  param
        ):
    ObjPricerUCP(scip, pp_name, instance, param)
{
    Master=M;

    int T= inst->getT() ;
    AlgoCplex = vector<CplexPricingAlgoTime*>(T, NULL) ;
    AlgoDynProg = vector<DynProgPricingAlgoTime*>(T, NULL) ;

    if (!Param.DynProgTime) {
        for (int t=0 ; t < T ; t++) {
            AlgoCplex[t] = new CplexPricingAlgoTime(inst, param, t) ;
        }
    }
    else {
        for (int t=0 ; t < T ; t++) {
            AlgoDynProg[t] = new DynProgPricingAlgoTime(inst, M, param, t) ;
        }
    }

    TimeSolNotFound = vector<int>(T,0) ;
    lastTimeStep=-1 ;
    nbCallsToCplex=0 ;
}


/** Destructs the pricer object. */
ObjPricerTimeUCP::~ObjPricerTimeUCP()
{
    cout<<"Destructeur du pricer"<<endl;
}

/** initialization method of variable pricer (called after problem was transformed)
 *
 *  Because SCIP transformes the original problem in preprocessing, we need to get the references to
 *  the variables and constraints in the transformed problem from the references in the original
 *  problem.
 */
SCIP_DECL_PRICERINIT(ObjPricerTimeUCP::scip_init)
{

    cout<<"**************PRICER INIT************ "<<endl;
    int T = inst->getT() ;
    int n = inst->getn();

    cout << Master->logical.size() << endl ;
    // logical constraints
    for (int i = 0 ; i < n ; i++) {
        for (int t = 1 ; t < T ; t++) {
        SCIPgetTransformedCons( scip, Master->logical.at(i*T+t), &(Master->logical.at(i*T+t)) );
        }
    }

    // min-up constraints
    for (int i = 0 ; i <n ; i++) {
        int L = inst->getL(i) ;
        for (int t = L ; t < T ; t++) {
            SCIPgetTransformedCons( scip, Master->min_up.at(i*T+t), &(Master->min_up.at(i*T+t)) );
        }
    }
    // min-down constraints
    for (int i = 0 ; i <n ; i++) {
        int l = inst->getl(i) ;
        for (int t = l ; t < T ; t++) {
            SCIPgetTransformedCons( scip, Master->min_down.at(i*T+t), &(Master->min_down.at(i*T+t)) );
        }
    }
    for (int t=0 ; t < T ; t++) {
        SCIPgetTransformedCons( scip, Master->convexity_cstr.at(t), &(Master->convexity_cstr.at(t)) );
    }

    cout<<"**************FIN PRICER INIT************ "<<endl;
    //variables ?
    //pour l'instant on n'a pas besoin de les manipuler a priori

    return SCIP_OKAY;
}


/** Pricing of additional variables if LP is feasible.
 *
 *  - get the values of the dual variables you need
 *  - construct the reduced units costs from dual costs
 *  - find the cheapest production plan for each site
 *  - if this plan has negative reduced cost, add it to the LP
 *
 *  possible return values for *result:
 *  - SCIP_SUCCESS    : at least one improving variable was found, or it is ensured that no such variable exists
 *  - SCIP_DIDNOTRUN  : the pricing process was aborted by the pricer, there is no guarantee that the current LP solution is optimal
 */
//SCIP_DECL_PRICERREDCOST(ObjPricerUCP::scip_redcost)
//{
//    SCIPdebugMsg(scip, "call scip_redcost ...\n");
//    /* set result pointer, see above */
//    *result = SCIP_SUCCESS;
//    /* call pricing routine */
//    pricingUCP(scip,0);
//    return SCIP_OKAY;
//}

SCIP_RETCODE ObjPricerTimeUCP::scip_redcost(SCIP* scip, SCIP_PRICER* pricer, SCIP_Real* lowerbound, SCIP_Bool* stopearly, SCIP_RESULT* result)
{

    SCIPdebugMsg(scip, "call scip_redcost ...\n");

    if( Param.PriceAndBranch && SCIPgetDepth(scip) != 0 )
    {
        *result = SCIP_SUCCESS;
        return SCIP_OKAY;
    }

    /* set result pointer, see above */
    *result = SCIP_SUCCESS;

    /* call pricing routine */
    pricingUCP(scip,0);

    return SCIP_OKAY;

}

SCIP_RETCODE ObjPricerTimeUCP::scip_farkas( SCIP* scip, SCIP_PRICER* pricer, SCIP_RESULT* result ){

    SCIPdebugMsg(scip, "call scip_farkas ...\n");

    if( Param.PriceAndBranch && SCIPgetDepth(scip) != 0 )
    {
        *result = SCIP_SUCCESS;
        return SCIP_OKAY;
    }


    /* set result pointer, see above */
    *result = SCIP_SUCCESS;

    /* call pricing routine */
    pricingUCP(scip,1);

    return SCIP_OKAY;

}



void ObjPricerTimeUCP::updateDualCosts(SCIP* scip, DualCostsTime & dual_cost, bool Farkas) {
    ///// RECUPERATION DES COUTS DUAUX

    int print = 0 ;
    int n = inst->getn() ;
    int T = inst->getT() ;

    //cout << "solution duale :" << endl ;

    //couts duaux "logical constraint"
    for (int i = 0; i < n; i++) {
        for (int t = 1 ; t < T ; t++) {
            if (!Farkas) {
                dual_cost.Mu.at(i*T+t) = SCIPgetDualsolLinear(scip, Master->logical.at(i*T+t));
            }
            else{
                dual_cost.Mu.at(i*T+t) = SCIPgetDualfarkasLinear(scip, Master->logical.at(i*T+t));
            }
            if (print)
                cout << "mu(" << i <<"," << t <<") = " << dual_cost.Mu[i*T+t] <<endl;
        }
    }

    //couts duaux "min-up constraint"
    for (int i = 0; i < n; i++) {
        int L = inst->getL(i) ;
        for (int t = L ; t < T ; t++) {
            if (!Farkas) {
                dual_cost.Nu.at(i*T+t) = SCIPgetDualsolLinear(scip, Master->min_up.at(i*T+t));
            }
            else{
                dual_cost.Nu.at(i*T+t) = SCIPgetDualfarkasLinear(scip, Master->min_up.at(i*T+t));
            }
            if (print)
                cout << "nu(" << i <<"," << t <<") = " << dual_cost.Nu.at(i*T+t) <<endl;
        }
    }

    //couts duaux "min-down constraint"
    for (int i = 0; i < n; i++) {
        int l = inst->getl(i) ;
        for (int t = l ; t < T ; t++) {
            if (!Farkas) {
                dual_cost.Xi.at(i*T+t) = SCIPgetDualsolLinear(scip, Master->min_down.at(i*T+t));
            }
            else{
                dual_cost.Xi.at(i*T+t) = SCIPgetDualfarkasLinear(scip, Master->min_down.at(i*T+t));
            }
            if (print)
                cout << "xi(" << i <<"," << t <<") = " << dual_cost.Xi.at(i*T+t) <<endl;
        }
    }

    //couts duaux "convexity constraint"
    for (int t = 0 ; t < T ; t++) {
        if (!Farkas) {
            dual_cost.Sigma.at(t) = SCIPgetDualsolLinear(scip, Master->convexity_cstr.at(t));
        }
        else{
            dual_cost.Sigma.at(t) = SCIPgetDualfarkasLinear(scip, Master->convexity_cstr.at(t));
        }
        if (print)
            cout << "sigma(" << t <<") = " << dual_cost.Sigma[t] <<endl;
    }


    // Couts duaux "interval up set"
    list<IneqIntUpSet*>::const_iterator iup;
    for (int t = 0 ; t < T ; t++) {
        for (iup = Master->IUP_t0[t].begin(); iup!= Master->IUP_t0[t].end() ; iup++) {
            if (!Farkas) {
                (*iup)->dual = SCIPgetDualsolLinear(scip, (*iup)->ineq);
            }
            else {
                (*iup)->dual = SCIPgetDualfarkasLinear(scip, (*iup)->ineq);
            }
        }
    }

    if (print) cout << endl ;

}

void ObjPricerTimeUCP::pricingUCP( SCIP*              scip  , bool Farkas             /**< SCIP data structure */)
{
#ifdef OUTPUT_PRICER
   //cout<<"**************PRICER************ " << endl ;
    // SCIPprintBestSol(scip, NULL, FALSE);
#endif

    int print = 0 ;

    //cout<<"**************PRICER************ " << endl ;



  // SCIPwriteTransProblem(scip, NULL, NULL, FALSE);
    //SCIPprintSol(scip, NULL, NULL, FALSE);
    if (print) {

//        /// PMR courant et sa solution
//        SCIPwriteTransProblem(scip, NULL, NULL, FALSE);

//        // cout << "solution du PMR:" << endl ;
//        SCIPprintSol(scip, NULL, NULL, FALSE);

//        //cout << "solution réalisable:" << endl ;
//        SCIPprintBestSol(scip, NULL, FALSE);
    }

    //// Cout duaux
    int T = inst->getT() ;
    int n = inst->getn() ;

    DualCostsTime dual_cost = DualCostsTime(inst) ;

    updateDualCosts(scip, dual_cost, Farkas);

    bool oneImprovingSolution = false ;

    int cas=0 ;
    int nb_cas=1;
    if (Param.DontPriceAllTimeSteps) {
        nb_cas=2 ;
    }

    int min=0 ;
    int max=T ;
    if (Param.OneTimeStepPerIter) {
        nb_cas=2 ;
    }

    while (!oneImprovingSolution && cas < nb_cas) { // Si on n'a pas trouvé de colonne améliorante dans le cas 1, on passe au cas 2: on cherche une colonne pour tous les pas de temps

        cas ++ ;

        if (Param.OneTimeStepPerIter) {
            lastTimeStep = (lastTimeStep+1)%T;
            min=lastTimeStep ;
            max=min+1 ;
        }

        //cout << "cas: " << cas << endl ;

        for (int t=min ; t < max ; t++) {

            if (print) cout << "time "<< t << endl;

            if (TimeSolNotFound.at(t) < 2 || cas==2 ) { // si un plan pour t a été généré au cours des 10 dernières itérations

                ///// MISE A JOUR DES OBJECTIFS DES SOUS PROBLEMES
                // cout << "mise à jour des couts, farkas=" << Farkas << endl;
                if (!Param.DynProgTime) {
                    (AlgoCplex.at(t))->updateObjCoefficients(inst, Param, dual_cost, Farkas) ;
                }
                else {
                    (AlgoDynProg.at(t))->updateObjCoefficients(inst, Param, dual_cost, Farkas) ;
                }

                //// CALCUL D'UN PLAN DE COUT REDUIT MINIMUM
                double objvalue = 0 ;
                double temps ;
                bool ImprovingSolutionFound;

                if (!Param.DynProgTime) {
                     ImprovingSolutionFound = (AlgoCplex.at(t))->findImprovingSolution(inst, dual_cost, objvalue, temps, cas-1);
                }
                else {
                    ImprovingSolutionFound = (AlgoDynProg.at(t))->findImprovingSolution(inst, dual_cost, objvalue, temps, cas-1);
                }
                nbCallsToCplex++;
                Master->cumul_resolution_pricing += temps ;

                if (ImprovingSolutionFound) {
                    oneImprovingSolution = true ;

                    TimeSolNotFound.at(t) = 0 ;

                    double realCost=0 ;
                    double totalProd=0 ;

                    IloNumArray upDownPlan ;
                    if (!Param.DynProgTime) {
                        upDownPlan = IloNumArray((AlgoCplex.at(t))->env, n) ;
                        (AlgoCplex.at(t))->getUpDownPlan(inst, dual_cost, upDownPlan, realCost, totalProd, Farkas) ;
                    }
                    else {
                        upDownPlan = IloNumArray((AlgoDynProg.at(t))->env, n) ;
                        (AlgoDynProg.at(t))->getUpDownPlan(inst, dual_cost, upDownPlan, realCost, totalProd, Farkas) ;
                    }

                     //

                    //cout << "total prod: " << totalProd << endl ;

                    // if (print) {
                         cout << "Minimum reduced cost plan: "<< objvalue << "for time " << t << endl ;
                         for (int i=0 ; i < n ; i++) {
                             cout << fabs(upDownPlan[i]) << " " ;
                         }
                         cout << endl ;
                     //}




                    int kmax = t ;
                    int kmin=t ;

                    if (Param.AddColumnToOtherTimeSteps) {
                        if (t < T-1) {
                            if (inst->getD(t) < inst->getD(t+1)) {
                                kmax = fmin(T-1, t+T/12) ;
                            }
                        }
                        if (t>0) {
                            if (inst->getD(t) < inst->getD(t-1)) {
                                kmin = fmax(0, t-T/12) ;
                            }
                        }
                    }

                    for (int k=kmin ; k <= kmax; k++) {

                        if (inst->getD(k) <= totalProd) {

                          //  cout << "ajout maitre" << endl;

                        /// AJOUT VARIABLE DANS LE MAITRE ////

                        MasterTime_Variable* lambda = new MasterTime_Variable(k, upDownPlan, realCost);
                        // cout << "Plan found for time " << t << " with reduced cost = " << objvalue << " ";
                        //// CREATION D'UNE NOUVELLE VARIABLE
                        Master->initMasterTimeVariable(scip, lambda) ;

                        /* add new variable to the list of variables to price into LP (score: leave 1 here) */
                        SCIPaddPricedVar(scip, lambda->ptr, 1.0);

                        ///// ADD COEFFICIENTS TO DEMAND, POWER LIMITS and CONVEXITY CONSTRAINTS
                        Master->addCoefsToConstraints(scip, lambda) ;
                        }

                    }

                }

                else {
                    if (Param.DontPriceAllTimeSteps) {
                        TimeSolNotFound.at(t) ++;
                    }
                }
            }
        }
    }

  //  cout<<"************END PRICER******************"<<endl;
#ifdef OUTPUT_PRICER
    SCIPwriteTransProblem(scip, "ucp.lp", "lp", FALSE);
  //  cout<<"************END PRICER******************"<<endl;
#endif

}

void ObjPricerTimeUCP::addVarBound(SCIP_ConsData* consdata) {

    int t = consdata->time ;
    int i = consdata->unit ;

    if (AlgoDynProg.at(t) != NULL) {
        if (consdata->bound == 0) {
            (AlgoDynProg.at(t))->W -= inst->getPmax(i) ;
            (AlgoDynProg.at(t))->init.at(i) = 0 ;
        }
        else {
            (AlgoDynProg.at(t))->init.at(i) = 1 ;
        }
    }
    else {
        cout << "AlgoDynProg.at(t) NULL: branchement non supporté pour sous problèmes résolus par Cplex dans la décomposition par pas de temps" << endl ;
    }
}

void ObjPricerTimeUCP::removeVarBound(SCIP_ConsData* consdata) {

    int t = consdata->time ;
    int i = consdata->unit ;

    if (AlgoDynProg.at(t) != NULL) {
        if (consdata->bound == 0) {
            (AlgoDynProg.at(t))->W += inst->getPmax(i) ;
        }

        (AlgoDynProg.at(t))->init.at(i) = -1 ;
    }
    else {
        cout << "AlgoDynProg.at(t) NULL: branchement non supporté pour sous problèmes résolus par Cplex dans la décomposition par pas de temps" << endl ;
    }
}


