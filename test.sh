#!/bin/bash
rm result.txt

nom=result.txt

sym=0

cat01=1
intra=1



dossier=data/small_UCP_data/

printf $dossier " \n" >> result.txt


printf " & n & T & D & id & IUP & Iter & Var & cols U & cols T & CPU & CPU(Master) & Gap & Dual b. & Primal b. & LR & LR(Cplex) \\\\\\ \n " >> result.txt



UseIntraCons=1

demand_type=3

for n in 20 ; do
  for T in 48 ; do
    for id in 1 2 3 4 5 ; do
      for met in 3011 ; do
        rm logs/$met.txt
        rm colonnes.csv
        rm convergence/${n}_${T}_$id.csv
        ./bin/SCIP_UCP_BP.linux.x86_64.gnu.opt.cpx 1 $dossier $n $T 1 $demand_type $sym $cat01 $intra $id $met $UseIntraCons >> logs/$met.txt
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