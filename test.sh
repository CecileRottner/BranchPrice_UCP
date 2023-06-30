#!/bin/bash
rm result.txt

nom=result.txt

sym=0
#sym=2

cat01=1
intra=1



dossier=data/small_UCP_data/
#dossier=data/Sym/
#dossier=data/debug_data/

printf $dossier " \n" >> result.txt


printf " & n & T & id & Iter & Var & cols U & cols T & CPU & CPU(Master) & Gap & Dual b. & Primal b. & LR & CPU(LR) & LR(Cplex) & CPU(LR Cplex)\\\\\\ \n " >> result.txt



UseIntraCons=0

demand_type=3

for n in 20 ; do
  for T in 48 ; do
    for id in {2..20} ; do
      for met in 10000 30000 30010 30020 30030 30040 30060 ; do
        rm logs/$met.txt
        rm colonnes.csv
        rm convergence/${n}_${T}_$id.csv
        ./bin/SCIP_UCP_BP.linux.x86_64.gnu.opt.cpx 1 $dossier $n $T 1 $demand_type $sym $cat01 $intra $id $met $UseIntraCons >> logs/$met.txt
        #python3 tracer.py $n $T $id $met
        #python3 convergence/plot.py $n $T $id $met
      done
      printf "\\hline \n" >> result.txt	
    done
   printf "\\hline \n" >> result.txt	
  done
  printf "\n" >> result.txt
  printf "\n" >> result.txt
done
printf "\n" >> result.txt
printf "\n" >> result.txt