#!/bin/bash
rm result.txt

nom=result.txt

sym=0

cat01=1
intra=1



dossier=data/small_UCP_data/
#dossier=data/debug_data/

printf $dossier " \n" >> result.txt


printf " & n & T & id & Nodes & CPU & CPU(Master) & Dual b. & Primal b. & CPLEX & Nodes & CPU & gap \\\\\\ \n " >> result.txt



UseIntraCons=0

demand_type=3

for n in 20 ; do
  for T in 24 ; do
    for id in 4; do
      for met in 31060 ; do
        rm logs/$met.txt
        rm colonnes.csv
        rm convergence/${n}_${T}_$id.csv
        rm profondeur/${n}_${T}_$id.csv
        ./bin/SCIP_UCP_BP.linux.x86_64.gnu.opt.cpx 1 $dossier $n $T 1 $demand_type $sym $cat01 $intra $id $met $UseIntraCons >> logs/$met.txt
        #python3 profondeur/plot.py $n $T $id $met
      done
      #python3 profondeur/moyennegenerale.py $n $T $met
      printf "\\hline \n" >> result.txt	
    done
   printf "\\hline \n" >> result.txt	
  done
  printf "\n" >> result.txt
  printf "\n" >> result.txt
done
printf "\n" >> result.txt
printf "\n" >> result.txt