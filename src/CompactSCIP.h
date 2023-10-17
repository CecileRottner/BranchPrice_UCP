#ifndef COMPACTSCIP
#define COMPACTSCIP

#include <vector>
#include <list>

/* scip includes */
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

/* user defined includes */
#include "InstanceUCP.h"
#include "Process.h"

/* namespace usage */
using namespace std;
using namespace scip;

class Compact_Model {
public:
    int n ;
    int T ;

    const Parameters Param ;
    InstanceUCP* inst ;

    // Keep a pointer on every constraint of the program
    vector<SCIP_CONS*> demand_cstr;
    vector<SCIP_CONS*> power_limits;
    vector<SCIP_CONS*> logical; // link x and u variables
    vector<SCIP_CONS*> min_up;
    vector<SCIP_CONS*> min_down;

    // Keep a pointer on each variable
    vector<SCIP_VAR*> x_var ;
    vector<SCIP_VAR*> p_var ;
    vector<SCIP_VAR*> u_var ;

    // Constraints specific to non-linear startup costs
    vector<SCIP_CONS*> downtime;
    vector<SCIP_CONS*> startup;

    // Variables specific to non-linear startup costs
    vector<SCIP_VAR*> u_temps_var ;

    Compact_Model(const Parameters & Par, InstanceUCP* i) ;

    virtual ~Compact_Model() {}

    void InitScipCompactModel(SCIP* scip, InstanceUCP* inst) ;
};
#endif /* COMPACT INCLUDED */