#include "BranchingRule.h"
#include "BranchConsHandler.h"

#define OUTPUT_BRANCHRULE

#define eps 1e-6

using namespace std;

SCIP_RETCODE BranchingRule::scip_execlp(SCIP* scip, SCIP_BRANCHRULE* branchrule, SCIP_Bool allowaddcons, SCIP_RESULT* result) {

#ifdef OUTPUT_BRANCHRULE
    cout << " --------------------- Branching Rule EXECLP ---------------  \n";
    cout << "Nombre de noeuds actuel : " << SCIPgetNNodes(scip) << std::endl;
#endif

    SCIP_ConsData *consdata;

    SCIP_NODE* node = SCIPgetCurrentNode(scip);


    if (node->conssetchg!=NULL) {
        consdata=SCIPconsGetData(node->conssetchg->addedconss[0]);
        //SCIP_CONSSETCHG* SCIP_Node::conssetchg --> constraint set changes at this node or NULL
        //typedef struct SCIP_ConsSetChg SCIP_CONSSETCHG --> tracks additions and removals of the set of active constraints
#ifdef OUTPUT_BRANCHRULE
        cout<<"Consdata non null"<<endl;
#endif
    }
    else {
        consdata=NULL;
#ifdef OUTPUT_BRANCHRULE
        cout<<"Consdata null"<<endl;
#endif

    }


    // Search for the "most fractional" unit

    int T = inst->getT() ;
    int n = inst->getn() ;
    int nbG = inst->getnbG() ;

    vector<double> group_frac = vector<double>(nbG*T, 0) ;
    vector<double> x_frac = vector<double>(n*T, 0) ;

    list<Master_Variable*>::const_iterator itv;
    SCIP_Real frac_value;

    for (itv = master->L_var.begin(); itv!=master->L_var.end(); itv++) {

        frac_value = fabs(SCIPgetVarSol(scip,(*itv)->ptr));

        int site = (*itv)->Site ;
        int first = inst->firstUnit(site) ;
        for (int i=0 ; i < inst->nbUnits(site) ; i++) {
            int group = inst->getGroup(first+i)  ;
            for (int t=0 ; t < T ; t++) {
                if ((*itv)->UpDown_plan[i*T+t] > eps) {
                    group_frac[group*T + t] += frac_value ;
                    x_frac[(first+i)*T+t] += frac_value ;
                }
            }
        }
    }


    SCIP_Real bestfrac = 1;
    SCIP_Real tmp;
    int group=0;
    int unit ;
    int time ;

    for (int i=0 ; i < n ; i++) {
        for (int t=0 ; t < T ; t++) {
            tmp = x_frac[i*T+t] ;
            if ( (tmp > eps ) && (tmp < 1-eps) && (fabs(tmp - 0.5) < fabs(bestfrac - 0.5) ) ) {
                bestfrac = tmp;
                unit = i ;
                time = t ;
            }
        }
    }



    if (bestfrac < 1 - eps) {

       // int unit = floor(bestfrac) + inst->getFirstG(group) ;

        int VarX=1 ;
        int Site = inst->getSiteOf(unit);
        int unit_on_site= unit - inst->firstUnit(Site) ;

#ifdef OUTPUT_BRANCHRULE
        cout<<"Branch on var x(" << unit <<", " << time << ") ";
        cout<<" of value : "<< x_frac[unit*T+time] <<endl;
#endif

        SCIP_NODE *newnode;
        SCIP_CONS *newcons;


        // first node
        SCIPcreateChild(scip, &newnode, 1000.0, SCIPgetLocalTransEstimate(scip));
        createBranchCstr(scip, VarX, 0, unit_on_site, time, Site, pricer, &newcons );
        SCIPaddConsNode(scip, newnode, newcons, NULL);
        SCIPreleaseCons(scip, &newcons);

        // second node
        SCIPcreateChild(scip, &newnode, 1000.0, SCIPgetLocalTransEstimate(scip));
        createBranchCstr(scip, VarX, 1, unit_on_site, time, Site, pricer, &newcons );
        SCIPaddConsNode(scip, newnode, newcons, NULL);
        SCIPreleaseCons(scip, &newcons);

        *result = SCIP_BRANCHED;
    }
    else{
        cout<<"Every variable is integer!!!!!!"<<endl;
        *result = SCIP_CUTOFF;
    }

#ifdef DEBUG
    cout << "\n*****END OF Branching Rule EXECLP ****\n";
    cout << "****************************************\n";
#endif
    
    return SCIP_OKAY;

}
