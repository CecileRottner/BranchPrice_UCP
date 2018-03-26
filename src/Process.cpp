#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <math.h>
#include <ctime>
#include <list>

#include "InstanceUCP.h"
#include "Process.h"

using namespace std ;

string to_string2(int number){
    string number_string = "";
    char ones_char = '0';
    int ones = 0;
    while(true){
        ones = number % 10;
        switch(ones){
        case 0: ones_char = '0'; break;
        case 1: ones_char = '1'; break;
        case 2: ones_char = '2'; break;
        case 3: ones_char = '3'; break;
        case 4: ones_char = '4'; break;
        case 5: ones_char = '5'; break;
        case 6: ones_char = '6'; break;
        case 7: ones_char = '7'; break;
        case 8: ones_char = '8'; break;
        case 9: ones_char = '9'; break;
        default : ; //ErrorHandling("Trouble converting number to string.");
        }
        number -= ones;
        number_string = ones_char + number_string;
        if(number == 0){
            break;
        }
        number = number/10;
    }
    return number_string;
}


string InstanceProcessed::createName() const {
    string s_n = to_string2(n) + "_" ;

    string s_T = s_n + to_string2(T) ;
    s_T = s_T + "_" ;

    string s_bloc = s_T + to_string2(bloc);
    s_bloc = s_bloc + "_" ;

    string s_demande = s_bloc + to_string2(demande) ;
    s_demande = s_demande + "_" ;

    string  s_symetrie = s_demande + to_string2(symetrie) ;
    s_symetrie = s_symetrie + "_" ;

    string s_cat = s_symetrie + to_string2(cat) ;
    s_cat = s_cat + "_" ;

    string s_intra = s_cat + to_string2(intra) ;
    s_intra = s_intra + "_" ;

    string s_id = s_intra + to_string2(id) ;
    string file = s_id + ".txt" ;

    return s_id ;
}
