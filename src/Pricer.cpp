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
ObjPricerSite::ObjPricerSite(
        SCIP*                                scip,          /**< SCIP pointer */
        const char*                         pp_name,      /**< name of pricer */
        MasterSite_Model*                        M,
        InstanceUCP*                        instance,
        const Parameters &                  param
        ):
    ObjPricerUCP(scip, pp_name, instance, param)
{
    Master=M ;
    AlgoCplex = vector<CplexPricingAlgo*>(Param.nbDecGpes, NULL) ;
    AlgoDynProg = vector<DynProgPricingAlgo*>(Param.nbDecGpes, NULL) ;
    iteration=0;

    if (!Param.DynProg) {
        for (int s=0 ; s < Param.nbDecGpes ; s++) {
            AlgoCplex[s] = new CplexPricingAlgo(inst, param, s) ;
        }
    }
    else {
        for (int s=0 ; s < Param.nbDecGpes ; s++) {
            AlgoDynProg.at(s) = new DynProgPricingAlgo(inst, Master, param, s) ;
        }
    }


}


/** Destructs the pricer object. */
ObjPricerSite::~ObjPricerSite()
{
    cout<<"Destructeur du pricer"<<endl;
}

/** initialization method of variable pricer (called after problem was transformed)
 *
 *  Because SCIP transformes the original problem in preprocessing, we need to get the references to
 *  the variables and constraints in the transformed problem from the references in the original
 *  problem.
 */
SCIP_DECL_PRICERINIT(ObjPricerSite::scip_init)
{
    //cout<<"**************PRICER INIT************ "<<endl;

    int T = inst->getT() ;
    // demand constraints
    for (int t = 0 ; t < T ; t++) {
        SCIPgetTransformedCons(scip, Master->demand_cstr.at(t), &(Master->demand_cstr.at(t)));
    }

    //power limits
    for (int t = 0 ; t < T ; t++) {
        for (int i = 0 ; i < inst->getn() ; i++) {
            SCIPgetTransformedCons(scip, Master->power_limits[i*T+t], &(Master->power_limits[i*T+t]));
        }
    }

    //convexity constraints
    for (int s = 0 ; s < Param.nbDecGpes ; s++) {
        SCIPgetTransformedCons(scip, Master->convexity_cstr[s], &(Master->convexity_cstr[s]));
    }


    if (Param.Ramp && Param.rampInMaster) {
        for (int t = 1; t < T; t++) {
            for (int i = 0 ; i < inst->getn() ; i++) {
                SCIPgetTransformedCons(scip, Master->ramp_up[i*T+t], &(Master->ramp_up[i*T+t]));
                SCIPgetTransformedCons(scip, Master->ramp_down[i*T+t], &(Master->ramp_down[i*T+t]));
            }
        }
    }

    if (Param.IntraSite && Param.UnitDecompo) {
        for (int t = 1; t < T; t++) {
            for (int s = 0 ; s < inst->getS() ; s++) {
                SCIPgetTransformedCons(scip, Master->intrasite[s*T+t], &(Master->intrasite[s*T+t]));
            }
        }
    }

    if (Param.StartUpDecompo) {
        for (int t = 0 ; t < T ; t++) {
            for (int i = 0 ; i < inst->getn() ; i++) {

                SCIPgetTransformedCons(scip, Master->mindown[i*T+t], &(Master->mindown[i*T+t]));

                SCIPgetTransformedCons(scip, Master->z_lambda[i*T+t], &(Master->z_lambda[i*T+t]));

                SCIPgetTransformedCons(scip, Master->logical[i*T+t], &(Master->logical[i*T+t]));

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

SCIP_RETCODE ObjPricerSite::scip_redcost(SCIP* scip, SCIP_PRICER* pricer, SCIP_Real* lowerbound, SCIP_Bool* stopearly, SCIP_RESULT* result)
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

SCIP_RETCODE ObjPricerSite::scip_farkas( SCIP* scip, SCIP_PRICER* pricer, SCIP_RESULT* result ){

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



void ObjPricerSite::updateDualCosts(SCIP* scip, DualCosts & dual_cost, bool Farkas) {
    ///// RECUPERATION DES COUTS DUAUX

    int print = 0 ;
    int n = inst->getn() ;
    int T = inst->getT() ;
    int S = Param.nbDecGpes ;

    //cout << "solution duale :" << endl ;
    //couts duaux des power limits
    for (int i = 0; i < n; i++) {
        for (int t = 0 ; t < T ; t++) {
            if (!Farkas) {
                dual_cost.Nu[i*T+t] = SCIPgetDualsolLinear(scip, Master->power_limits[i*T+t]);
            }
            else{
                dual_cost.Nu[i*T+t] = SCIPgetDualfarkasLinear(scip, Master->power_limits[i*T+t]);
            }
            if (print)
                cout << "nu(" << i <<"," << t <<") = " << dual_cost.Nu[i*T+t] <<endl;
        }
    }

    //couts duaux des ramp
    //RAMPSTUFF
    if (Param.Ramp && Param.rampInMaster) {
        for (int i = 0; i < n; i++) {
            for (int t = 1 ; t < T ; t++) {
                if (!Farkas) {
                    dual_cost.Phi[i*T+t] = SCIPgetDualsolLinear(scip, Master->ramp_up[i*T+t]);
                    dual_cost.Psi[i*T+t] = SCIPgetDualsolLinear(scip, Master->ramp_down[i*T+t]);
                }
                else{
                    dual_cost.Phi[i*T+t] = SCIPgetDualfarkasLinear(scip, Master->ramp_up[i*T+t]);
                    dual_cost.Psi[i*T+t] = SCIPgetDualfarkasLinear(scip, Master->ramp_down[i*T+t]);
                }

               if (print) cout << "phi(" << i <<"," << t <<") = " << dual_cost.Phi[i*T+t] <<endl;
               if (print) cout << "psi(" << i <<"," << t <<") = " << dual_cost.Psi[i*T+t] <<endl;
            }
        }
    }
    //couts duaux demande
    for (int t = 0 ; t < T ; t++) {
        if (!Farkas) {
            dual_cost.Mu[t] = SCIPgetDualsolLinear(scip, Master->demand_cstr[t]);
        }
        else{
            dual_cost.Mu[t] = SCIPgetDualfarkasLinear(scip, Master->demand_cstr[t]);
        }
        if (print) cout << "mu: " << dual_cost.Mu[t] <<endl;
    }

    //couts duaux contrainte convexité
    for (int s = 0 ; s < S ; s++) {
        if (!Farkas) {
            dual_cost.Sigma[s] = SCIPgetDualsolLinear(scip, Master->convexity_cstr[s]);
        }
        else{
            dual_cost.Sigma[s] = SCIPgetDualfarkasLinear(scip, Master->convexity_cstr[s]);
        }
        if (print) cout << "sigma: " << dual_cost.Sigma[s] <<endl;
    }

    //couts duaux intrasite
    if (Param.IntraSite && Param.UnitDecompo) {
        for (int s = 0 ; s < inst->getS() ; s++) {
            for (int t = 1 ; t < T ; t++) {
                if (!Farkas) {
                    dual_cost.Eta[s*T+t] = SCIPgetDualsolLinear(scip, Master->intrasite[s*T+t]);
                }
                else{
                    dual_cost.Eta[s*T+t] = SCIPgetDualfarkasLinear(scip, Master->intrasite[s*T+t]);
                }
            }
        }
    }


    //couts duaux des contraintes pour Start up decompo
    if (Param.StartUpDecompo) {
        for (int i = 0; i < n; i++) {
            for (int t = 0 ; t < T ; t++) {

                if (!Farkas) {
                    if (t >= inst->getL(i)) {
                        dual_cost.Zeta[i*T+t] = SCIPgetDualsolLinear(scip, Master->mindown[i*T+t]);
                    }

                    dual_cost.Ksi[i*T+t] = SCIPgetDualsolLinear(scip, Master->z_lambda[i*T+t]);

                    if (t>= 1) {
                        dual_cost.Theta[i*T+t] = SCIPgetDualsolLinear(scip, Master->logical[i*T+t]);
                    }

                    if (print) cout << "zeta(" << i <<"," << t <<") = " << dual_cost.Zeta[i*T+t] <<endl;
                    if (print) cout << "ksi(" << i <<"," << t <<") = " << dual_cost.Ksi[i*T+t] <<endl;
                    if (print) cout << "theta(" << i <<"," << t <<") = " << dual_cost.Theta[i*T+t] <<endl;
                }
                else{
                    if (t>= inst->getL(i)) {
                        dual_cost.Zeta[i*T+t] = SCIPgetDualfarkasLinear(scip, Master->mindown[i*T+t]);
                    }

                    dual_cost.Ksi[i*T+t] = SCIPgetDualfarkasLinear(scip, Master->z_lambda[i*T+t]);

                    if (t>= 1) {
                        dual_cost.Theta[i*T+t] = SCIPgetDualfarkasLinear(scip, Master->logical[i*T+t]);
                    }
                }

            }
        }
    }
    if (print) cout << endl ;

}

void ObjPricerSite::pricingUCP( SCIP*              scip  , bool Farkas             /**< SCIP data structure */)
{
#ifdef OUTPUT_PRICER
    cout<<"**************PRICER************ "<< endl ;
    // SCIPprintBestSol(scip, NULL, FALSE);
#endif

    int print = 1 ;
    iteration++;

    //int iteration_limit=5 ;
//    /// PMR courant et sa solution
   // SCIPwriteTransProblem(scip, NULL, NULL, FALSE);

//   // cout << "solution du PMR:" << endl ;
//    SCIPprintSol(scip, NULL, NULL, FALSE);

//    //cout << "solution réalisable:" << endl ;
//    SCIPprintBestSol(scip, NULL, FALSE);

    //// Cout duaux
    int T = inst->getT() ;
    int S = Param.nbDecGpes ;

    DualCosts dual_cost = DualCosts(inst,Param) ;
    updateDualCosts(scip, dual_cost, Farkas);
    dual_cost.computeObjCoef(inst,Param,Farkas);

    double epsilon= 0.0000001 ;
    for (int s = 0 ; s < S ; s++) {

       //cout << "site "<< s << endl;

        ///// MISE A JOUR DES OBJECTIFS DES SOUS PROBLEMES
       // cout << "mise à jour des couts, farkas=" << Farkas << endl;
        if (!Param.DynProg) {
            (AlgoCplex[s])->updateObjCoefficients(inst, Param, dual_cost, Farkas) ;
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
            upDownPlan = IloNumArray((AlgoCplex[s])->env, Param.nbUnits(s)*T) ;
            solutionFound= (AlgoCplex[s])->findUpDownPlan(inst, dual_cost, upDownPlan, objvalue) ;
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

            cout << "ici" << endl ;
            upDownPlan = IloNumArray((AlgoDynProg[s])->env, Param.nbUnits(s)*T) ;
            (AlgoDynProg.at(s))->findImprovingSolution(inst, dual_cost, objvalue);
            (AlgoDynProg.at(s))->getUpDownPlan(inst, upDownPlan) ;

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
        }

        cout << endl ;

        //if (SCIPisNegative(scip, objvalue)) {

        if (objvalue < -epsilon ) {

            Master_Variable* lambda = new Master_Variable(s, upDownPlan);
            cout << "Plan found for site " << s << " with reduced cost = " << objvalue << " "  << endl ;

            if (Param.powerPlanGivenByLambda && !Param.DynProg) {
                powerPlan = IloNumArray((AlgoCplex[s])->env, Param.nbUnits(s)*T) ;
                (AlgoCplex[s]->cplex).getValues(AlgoCplex[s]->p, powerPlan) ;
                cout << "power plan: " << powerPlan << endl;
                lambda->addPowerPlan(powerPlan);
            }

            //// CREATION D'UNE NOUVELLE VARIABLE DANS LE MASTER
            Master->initMasterVariable(scip, inst, lambda) ;

            /* add new variable to the list of variables to price into LP (score: leave 1 here) */
            SCIPaddPricedVar(scip, lambda->ptr, 1.0);

            ///// ADD COEFFICIENTS TO DEMAND, POWER LIMITS and CONVEXITY CONSTRAINTS
            Master->addCoefsToConstraints(scip, lambda, inst) ;
        }
    }

#ifdef OUTPUT_PRICER
    SCIPwriteTransProblem(scip, "ucp.lp", "lp", FALSE);
    cout<<"************END PRICER******************"<<endl;
#endif

}

void ObjPricerSite::addVarBound(SCIP_ConsData* consdata) {

    cout << "Enter addVarBound:" << endl;

    if (!Param.DynProg) {
        AlgoCplex[consdata->site]->model.add(consdata->BranchConstraint) ;
    }
    else {
        (AlgoDynProg[consdata->site])->branchingDecisions.at(consdata->time) = consdata->bound ;
        cout << "for unit " << consdata->site << ", at time " << consdata->time <<", bound set to " << consdata->bound << endl ;
        cout << "End addVarBound" << endl ;
    }
}

void ObjPricerSite::removeVarBound(SCIP_ConsData* consdata) {

    if (!Param.DynProg) {
        AlgoCplex[consdata->site]->model.remove(consdata->BranchConstraint) ;
    }
    else {
        (AlgoDynProg[consdata->site])->branchingDecisions.at(consdata->time) = 8 ;
    }
}


