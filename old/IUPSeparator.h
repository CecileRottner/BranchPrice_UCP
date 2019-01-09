#ifndef IUPSEPA
#define IUPSEPA


#include "scip/scip.h"


class IUPHandler : public scip::ObjSepa {

public :

    MasterTime_Model* Master ;
    InstanceUCP* inst ;

    IUPHandler(SCIP* scip, MasterTime_Model* M, InstanceUCP* i) :
        scip::ObjSepa(
            scip,
            "IUPSepa",                    // const char *  	name,
            "Separator For Interval Up Set inequalities",   // const char *  	desc,
            2000000, -2000000, -2000000,           // int sepapriority, int enfopriority, int checkpriority,
            1, -1, 1, 0,                           // int sepafreq, int propfreq, int eagerfreq, int maxprerounds,
            FALSE, FALSE, FALSE,                   // delaysepa, delayprop, needscons,
            SCIP_PROPTIMING_BEFORELP,              // SCIP_PROPTIMING  	proptiming,
            SCIP_PRESOLTIMING_FAST                 // SCIP_PRESOLTIMING  	presoltiming
            )

    {
        Master = M ;
        inst = i ;
    }


    SCIP_RETCODE SCIPincludeSepaGomory(
            SCIP*                 scip                /**< SCIP data structure */
    );


}
#endif
