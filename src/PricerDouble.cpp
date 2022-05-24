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
ObjPricerDouble::ObjPricerDouble(
        SCIP*                                scip,          /**< SCIP pointer */
        const char*                         pp_name,      /**< name of pricer */
        MasterDouble_Model*                        M,
        InstanceUCP*                        instance,
        const Parameters &                  param
        ):
    ObjPricerUCP(scip, pp_name, instance, param)
{
    Master=M ;
    //cout<<Master->S<<endl;
    AlgoCplex_site = vector<CplexPricingAlgo*>(Param.nbDecGpes, NULL) ;
    AlgoDynProg_site = vector<DynProgPricingAlgo*>(Param.nbDecGpes, NULL) ;
    iteration=0;

    unitColumns=0;
    timeColumns=0;

    if (!Param.DynProg) {
        for (int s=0 ; s < Param.nbDecGpes ; s++) {
            AlgoCplex_site.at(s) = new CplexPricingAlgo(inst, param, s) ;
        }
    }
    else {
        for (int s=0 ; s < Param.nbDecGpes ; s++) {
            AlgoDynProg_site.at(s) = new DynProgPricingAlgo(inst, Master, param, s) ;
            // cout << "s = " << s << endl;
            // cout << AlgoDynProg_site.at(s)->Site << endl;
        }
    }


    int T= inst->getT() ;
    AlgoCplex_time = vector<CplexPricingAlgoTime*>(T, NULL) ;
    AlgoDynProg_time = vector<DynProgPricingAlgoTime*>(T, NULL) ;

    timeStepColumns = vector<int>(T,0);

    if (!Param.DynProgTime) {
        for (int t=0 ; t < T ; t++) {
            AlgoCplex_time.at(t) = new CplexPricingAlgoTime(inst, param, t) ;
        }
    }
    else {
        if (Param.powerPlanGivenByMu) {
            for (int t=0 ; t < T ; t++) {
                AlgoDynProg_time.at(t) = new DynProgPricingAlgoTimePower(inst, Master, param, t) ;
            }
        }
        else {
            for (int t=0 ; t < T ; t++) {
                AlgoDynProg_time.at(t) = new DynProgPricingAlgoTimeNoPower(inst, Master, param, t) ;
            }
        }
    }
}


/** Destructs the pricer object. */
ObjPricerDouble::~ObjPricerDouble()
{
    cout<<"Destructeur du pricer"<<endl;
}

/** initialization method of variable pricer (called after problem was transformed)
 *
 *  Because SCIP transformes the original problem in preprocessing, we need to get the references to
 *  the variables and constraints in the transformed problem from the references in the original
 *  problem.
 */
SCIP_DECL_PRICERINIT(ObjPricerDouble::scip_init)
{
    cout<<"**************PRICER INIT************ "<<endl;

    int T = inst->getT() ;
    int n = inst->getn() ;

    //cout<<"site convexity"<<endl;
    //cout<<Master->S<<endl;
    //site convexity constraints
    for (int s = 0 ; s < Param.nbDecGpes ; s++) {
        //cout<<Master->conv_lambda_site.at(s)<<endl;
        SCIPgetTransformedCons(scip, Master->conv_lambda_site.at(s), &(Master->conv_lambda_site.at(s)));
    }

    //cout<<"time convexity"<<endl;
    // time convexity constraints
    for (int t = 0 ; t < T ; t++) {
        SCIPgetTransformedCons(scip, Master->conv_lambda_time.at(t), &(Master->conv_lambda_time.at(t)));
    }
    //cout<<"equality"<<endl;
    // equality time / site
    for (int t = 0 ; t < T ; t++) {
        for (int i = 0 ; i < n ; i++) {
            SCIPgetTransformedCons(scip, Master->eq_time_site.at(i*T+t), &(Master->eq_time_site.at(i*T+t)));
        }
    }

    if (Param.minUpDownDouble) {
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
    }



    if (Param.IntraSite && Param.UnitDecompo) {
        for (int t = 1; t < T; t++) {
            for (int s = 0 ; s < inst->getS() ; s++) {
                //SCIPgetTransformedCons(scip, Master->intrasite[s*T+t], &(Master->intrasite[s*T+t]));
            }
        }
    }


    cout<<"**************FIN PRICER INIT************ "<<endl ;
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
//SCIP_DECL_PRICERREDCOST(ObjPricerSite::scip_redcost)
//{
//    SCIPdebugMsg(scip, "call scip_redcost ...\n");
//    /* set result pointer, see above */
//    *result = SCIP_SUCCESS;
//    /* call pricing routine */
//    pricingUCP(scip,0);
//    return SCIP_OKAY;
//}

SCIP_RETCODE ObjPricerDouble::scip_redcost(SCIP* scip, SCIP_PRICER* pricer, SCIP_Real* lowerbound, SCIP_Bool* stopearly, SCIP_RESULT* result)
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

SCIP_RETCODE ObjPricerDouble::scip_farkas( SCIP* scip, SCIP_PRICER* pricer, SCIP_RESULT* result ){

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



void ObjPricerDouble::updateDualCosts_site(SCIP* scip, DualCosts & dual_cost, bool Farkas) {
    ///// RECUPERATION DES COUTS DUAUX


    int print = 0 ;
    int n = inst->getn() ;
    int T = inst->getT() ;
    int S = Param.nbDecGpes ;

    //cout << "solution duale :" << endl ;
    //couts duaux contrainte égalité
    for (int i = 0; i < n; i++) {
        for (int t = 0 ; t < T ; t++) {
            if (!Farkas) {
                dual_cost.Omega[i*T+t] = SCIPgetDualsolLinear(scip, Master->eq_time_site.at(i*T+t) );
            }
            else{
                dual_cost.Omega[i*T+t] = SCIPgetDualfarkasLinear(scip, Master->eq_time_site.at(i*T+t) );
            }

            if (print)
                cout << "omega(" << i <<"," << t <<") = " << dual_cost.Omega[i*T+t] <<endl;
        }
    }


    //couts duaux contrainte convexité site
    for (int s = 0 ; s < S ; s++) {

        if (!Farkas) {
            dual_cost.Sigma[s] = SCIPgetDualsolLinear(scip, Master->conv_lambda_site[s]);
        }
        else{
            dual_cost.Sigma[s] = SCIPgetDualfarkasLinear(scip, Master->conv_lambda_site[s]);
        }
        if (print) cout << "sigma: " << dual_cost.Sigma[s] <<endl;
    }

    if (Param.PminDifferentPmax && !Param.powerPlanGivenByMu){
        //couts duaux power limits
        for (int i = 0; i < n; i++) {
            for (int t = 0 ; t < T ; t++) {
                if (!Farkas) {
                    dual_cost.Nu.at(i*T+t) = SCIPgetDualsolLinear(scip, Master->power_limits.at(i*T+t));
                }
                else{
                    dual_cost.Nu.at(i*T+t) = SCIPgetDualfarkasLinear(scip, Master->power_limits.at(i*T+t));
                }
                if (print)
                    cout << "nu(" << i <<"," << t <<") = " << dual_cost.Nu.at(i*T+t) <<endl;
            }
        }
    }

    //couts duaux intrasite
    if (Param.IntraSite && Param.UnitDecompo) {
        //        for (int s = 0 ; s < inst->getS() ; s++) {
        //            for (int t = 1 ; t < T ; t++) {
        //                if (!Farkas) {
        //                    dual_cost.Eta[s*T+t] = SCIPgetDualsolLinear(scip, Master->intrasite[s*T+t]);
        //                }
        //                else{
        //                    dual_cost.Eta[s*T+t] = SCIPgetDualfarkasLinear(scip, Master->intrasite[s*T+t]);
        //                }
        //            }
        //        }
    }



}
void ObjPricerDouble::updateDualCosts_time(SCIP* scip, DualCostsTime & dual_cost, bool Farkas) {
    ///// RECUPERATION DES COUTS DUAUX

    int print = 0 ;
    int n = inst->getn() ;
    int T = inst->getT() ;
    int S = Param.nbDecGpes ;

    //cout << "solution duale :" << endl ;
    //couts duaux contrainte égalité
    for (int i = 0; i < n; i++) {
        for (int t = 0 ; t < T ; t++) {
            if (!Farkas) {
                dual_cost.Omega[i*T+t] = SCIPgetDualsolLinear(scip, Master->eq_time_site.at(i*T+t) );
            }
            else{
                dual_cost.Omega[i*T+t] = SCIPgetDualfarkasLinear(scip, Master->eq_time_site.at(i*T+t) );
            }
            if (print)
                cout << "omega(" << i <<"," << t <<") = " << dual_cost.Omega[i*T+t] <<endl;
        }
    }


    //couts duaux "time convexity constraint"
    for (int t = 0 ; t < T ; t++) {
        if (!Farkas) {
            dual_cost.Sigma.at(t) = SCIPgetDualsolLinear(scip, Master->conv_lambda_time.at(t));
        }
        else{
            dual_cost.Sigma.at(t) = SCIPgetDualfarkasLinear(scip, Master->conv_lambda_time.at(t));
        }
        if (print)
            cout << "sigma(" << t <<") = " << dual_cost.Sigma[t] <<endl;
    }

    if (Param.minUpDownDouble) {
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
    }


}

void ObjPricerDouble::pricingUCP( SCIP*              scip  , bool Farkas             /**< SCIP data structure */)
{
#ifdef OUTPUT_PRICER
    cout<<"**************PRICER************ "<< endl ;
    // SCIPprintBestSol(scip, NULL, FALSE);
#endif

    Master->nbIter++;

    totalDualCost = 0;
    int T = inst->getT() ;
    int n = inst->getn() ;

    int print = 1;
    iteration++;

    //int iteration_limit=5 ;
    //    /// PMR courant et sa solution
    // SCIPwriteTransProblem(scip, NULL, NULL, FALSE);

    //   // cout << "solution du PMR:" << endl ;
    //    SCIPprintSol(scip, NULL, NULL, FALSE);

    //    //cout << "solution réalisable:" << endl ;
    //    SCIPprintBestSol(scip, NULL, FALSE);

    //// Cout duaux
    int S = Param.nbDecGpes ;

////////// MISE A JOUR DES COUTS DUAUX

// Cout duaux de time à mettre à jour avant ceux de Site, car la méthode computeObjCoef prend en arg dual_cost_time 
    DualCostsTime dual_cost_time = DualCostsTime(inst) ;
    updateDualCosts_time(scip, dual_cost_time, Farkas);
    DualCosts dual_cost = DualCosts(inst,Param) ;
    updateDualCosts_site(scip, dual_cost, Farkas);
    dual_cost.computeObjCoef(inst,Param,Farkas, dual_cost_time);

    /////Recherche variable améliorante de type sites
    double epsilon= 0.0000001 ;
    for (int s = 0 ; s < S ; s++) {

        //cout << "site "<< s << endl;

        ///// MISE A JOUR DES OBJECTIFS DES SOUS PROBLEMES
        // cout << "mise à jour des couts, farkas=" << Farkas << endl;
        if (!Param.DynProg) {
            (AlgoCplex_site[s])->updateObjCoefficients(inst, Param, dual_cost, Farkas) ;
        }
        else {
            //nothing to do
            // (AlgoDynProg[s])->updateObjCoefficients(inst, Param, dual_cost, Farkas) ;

        }

        //// CALCUL D'UN PLAN DE COUT REDUIT MINIMUM
        double objvalue = 0 ;
        IloNumArray upDownPlan  ;
        IloNumArray powerPlan  ;
        int solutionFound ;


        if (!Param.DynProg) {
            upDownPlan = IloNumArray((AlgoCplex_site[s])->env, Param.nbUnits(s)*T) ;
            solutionFound= (AlgoCplex_site[s])->findUpDownPlan(inst, dual_cost, upDownPlan, objvalue) ;
            for (int index=0 ; index <Param.nbUnits(s)*T ; index++ ) {
                if (upDownPlan[index]>1-epsilon) {
                    upDownPlan[index]=1 ;
                }
                if (upDownPlan[index]< epsilon) {
                    upDownPlan[index]=0 ;
                }
            }
        }

        else { // résolution par programmation dynamique
            cout << "début dynprogsite" << endl;
            cout << "s = " << s << endl;
            cout << AlgoDynProg_site[s]->Site << endl;
            upDownPlan = IloNumArray((AlgoDynProg_site[s])->env, Param.nbUnits(s)*T) ;
            cout << "updownplan initialisé" << endl;
            if (Param.DynProgSUSD) {
                (AlgoDynProg_site.at(s))->findImprovingSolutionSUSD(inst, dual_cost, objvalue);
                (AlgoDynProg_site.at(s))->getUpDownPlanSUSD(inst, upDownPlan) ;
            }
            else {
                (AlgoDynProg_site.at(s))->findImprovingSolution(inst, dual_cost, objvalue);
                (AlgoDynProg_site.at(s))->getUpDownPlan(inst, upDownPlan) ;
            }
            cout << "DP resolution done" << endl ;

        }

        // cout << "solution found: " << solutionFound << endl;
        if (!solutionFound) {
            //PRUNE THE NODE
        }
        if (print) cout << "Minimum reduced cost plan: "<< objvalue << endl ;

        if (print) {
            for (int t=0 ; t < T ; t++)  {
                for (int i=0 ; i < Param.nbUnits(s) ; i++) {
                    cout << fabs(upDownPlan[i*T+t]) << " " ;
                }
                //cout << endl ;
            }
            cout << endl ;
        cout << endl ;
        }


        //if (SCIPisNegative(scip, objvalue)) {

        if (objvalue < -epsilon ) {

            Master_Variable* lambda = new Master_Variable(s, upDownPlan);
            if (print) cout << "Plan found for site " << s << " with reduced cost = " << objvalue << " "  << endl ;
            totalDualCost += objvalue;
            unitColumns++;

            if (Param.powerPlanGivenByLambda && !Param.DynProg) {
                powerPlan = IloNumArray((AlgoCplex_site[s])->env, Param.nbUnits(s)*T) ;
                (AlgoCplex_site[s]->cplex).getValues(AlgoCplex_site[s]->p, powerPlan) ;
                if (print) cout << "power plan: " << powerPlan << endl;
                lambda->addPowerPlan(powerPlan);
            }

            SCIPwriteOrigProblem(scip, "debug.lp", "lp", FALSE);

            //// CREATION D'UNE NOUVELLE VARIABLE DANS LE MASTER
            Master->initMasterSiteVariable(scip, inst, lambda) ;

            /* add new variable to the list of variables to price into LP (score: leave 1 here) */
            SCIP_RETCODE ajout = SCIPaddPricedVar(scip, lambda->ptr, 1.0);
            cout << "ajout var par unité: " << ajout << endl;

            ///// ADD COEFFICIENTS TO CONVEXITY and TIME/SITE EQUALITY CONSTRAINTS
            Master->addCoefsToConstraints_siteVar(scip, lambda, inst) ;

            SCIPwriteOrigProblem(scip, "debug.lp", "lp", FALSE);

            cout << "redcost: " << SCIPgetVarRedcost(scip, lambda->ptr) << endl;
        }
    }

    if (print) cout << "RECHERCHE SUR LES PAS DE TEMPS" << endl ;


    /////Recherche variable améliorante de type time

    //// Cout duaux


    //cout << "cas: " << cas << endl ;

    for (int t=0 ; t < T ; t++) {

        if (print) cout << "time "<< t << endl;


        ///// MISE A JOUR DES OBJECTIFS DES SOUS PROBLEMES
        // cout << "mise à jour des couts, farkas=" << Farkas << endl;
        if (!Param.DynProgTime) {
            cout << "début updateobjcoeff" << endl;
            (AlgoCplex_time.at(t))->updateObjCoefficients(inst, Param, dual_cost_time, Farkas) ;
            if(t == 0 && Master->nbIter == 2){
                (AlgoCplex_time.at(t))->cplex.exportModel( ( std::to_string(Param.PminDifferentPmax) + "_bug.lp" ).c_str() );
                for (int i=0 ; i<n ; i++) {
                    cout << "i:" << i << endl;
                    cout << dual_cost_time.Omega.at(i*T+t) << endl;
                }
            }
        }
        else {
            cout << "début updateobjcoeff" << endl;
            (AlgoDynProg_time.at(t))->updateObjCoefficients(inst, Param, dual_cost_time, Farkas) ;
        }

        //// CALCUL D'UN PLAN DE COUT REDUIT MINIMUM
        double objvalue = 0 ;
        double temps ;
        bool ImprovingSolutionFound;

        if (!Param.DynProgTime) {
            cout << "début findimprovingsol" << endl;
            ImprovingSolutionFound = (AlgoCplex_time.at(t))->findImprovingSolution(inst, dual_cost_time, objvalue, temps, 1);
        }
        else {
            cout << "début findimprovingsol" << endl;
            ImprovingSolutionFound = (AlgoDynProg_time.at(t))->findImprovingSolution(inst, dual_cost_time, objvalue, temps, Param.heurPricingTime);
        }
        Master->cumul_resolution_pricing += temps ;

        if (ImprovingSolutionFound) {

            timeColumns++;

            timeStepColumns.at(t) += 1;

            double realCost=0 ;
            double totalProd=0 ;

            IloNumArray upDownPlan  ;
            IloNumArray powerPlan  ;

            if (Param.DynProgTime){
                upDownPlan = IloNumArray((AlgoDynProg_time.at(t))->env, n) ;
                powerPlan = IloNumArray((AlgoDynProg_time.at(t))->env, n) ;

                (AlgoDynProg_time.at(t))->getUpDownPlan(inst, dual_cost_time, upDownPlan, powerPlan, realCost, totalProd, Farkas) ;
            }

            else{
                upDownPlan = IloNumArray((AlgoCplex_time.at(t))->env, n) ;
                powerPlan = IloNumArray((AlgoCplex_time.at(t))->env, n) ;

                (AlgoCplex_time.at(t))->getUpDownPlan(inst, dual_cost_time, upDownPlan, powerPlan, realCost, totalProd, Farkas) ;
            }


            if (print) {
                cout << "Minimum reduced cost plan: "<< objvalue << "for time " << t << endl ;
                for (int i=0 ; i < n ; i++) {
                    cout << fabs(upDownPlan[i]) << " " ;
                }
                cout << endl ;
            }

            totalDualCost += objvalue;

            //cout << "total prod: " << totalProd << endl ;

            /// AJOUT VARIABLE DANS LE MAITRE ////

            MasterTime_Variable* lambda = new MasterTime_Variable(t, upDownPlan);
            // cout << "Plan found for time " << t << " with reduced cost = " << objvalue << " ";

            if (Param.powerPlanGivenByMu) {
                lambda->addPowerPlan(powerPlan);
            }

            SCIPwriteOrigProblem(scip, "debug.lp", "lp", FALSE);

            //// CREATION D'UNE NOUVELLE VARIABLE
            Master->initMasterTimeVariable(scip, lambda) ;

            /* add new variable to the list of variables to price into LP (score: leave 1 here) */
            SCIP_RETCODE ajout = SCIPaddPricedVar(scip, lambda->ptr, 1.0);
            cout << "ajout var par temps: " << ajout << endl;

            ///// ADD COEFFICIENTS TO DEMAND, POWER LIMITS and CONVEXITY CONSTRAINTS
            Master->addCoefsToConstraints_timeVar(scip, lambda) ;

            SCIPwriteOrigProblem(scip, "debug.lp", "lp", FALSE);

            cout << "redcost: " << SCIPgetVarRedcost(scip, lambda->ptr) << endl;
        }
    }

    cout << "total: " << totalDualCost << endl;

    Master->totalDualCostList.push_back(totalDualCost);


#ifdef OUTPUT_PRICER
    SCIPwriteTransProblem(scip, "ucp.lp", "lp", FALSE);
    cout<<"************END PRICER******************"<<endl;
#endif

}

void ObjPricerDouble::addVarBound(SCIP_ConsData* consdata) {

    int t = consdata->time ;
    int i = consdata->unit ;

    cout << "Enter addVarBound:" << endl;

    if (!Param.DynProg) {
        AlgoCplex_site[consdata->site]->model.add(consdata->BranchConstraint) ;
    }
    else {
        (AlgoDynProg_site[consdata->site])->branchingDecisions.at(t) = consdata->bound ;
        cout << "for unit " << consdata->site << ", at time " << t <<", bound set to " << consdata->bound << endl ;
        cout << "End addVarBound" << endl ;
    }

    if (AlgoDynProg_time.at(t) != NULL) {
        if (consdata->bound == 0) {
            if (!Param.powerPlanGivenByMu) {
                (AlgoDynProg_time.at(t))->W -= inst->getPmax(i) ;
            }
            (AlgoDynProg_time.at(t))->init.at(i) = 0 ;
        }
        else {
            (AlgoDynProg_time.at(t))->init.at(i) = 1 ;
        }
    }
    else {
        cout << "AlgoDynProg.at(t) NULL: branchement non supporté pour sous problèmes résolus par Cplex dans la décomposition par pas de temps" << endl ;
    }
}

void ObjPricerDouble::removeVarBound(SCIP_ConsData* consdata) {

    int t = consdata->time ;
    int i = consdata->unit ;

    if (!Param.DynProg) {
        AlgoCplex_site[consdata->site]->model.remove(consdata->BranchConstraint) ;
    }
    else {
        (AlgoDynProg_site[consdata->site])->branchingDecisions.at(t) = 8 ;
    }

    if (AlgoDynProg_time.at(t) != NULL) {
        if (consdata->bound == 0 && !Param.powerPlanGivenByMu) {
            (AlgoDynProg_time.at(t))->W += inst->getPmax(i) ;
        }

        (AlgoDynProg_time.at(t))->init.at(i) = -1 ;
    }
    else {
        cout << "AlgoDynProg.at(t) NULL: branchement non supporté pour sous problèmes résolus par Cplex dans la décomposition par pas de temps" << endl ;
    }
}


