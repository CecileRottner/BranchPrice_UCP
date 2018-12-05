#!/bin/bash
rm result.txt

nom=result.txt

sym=2

cat01=1
intra=1



dossier=data/Data_/Sym/

printf $dossier " \n" >> result.txt


printf " & n & T & D & id & IUP & Iter & Var & CPU & CPU(Master) & Gap & Dual b. & LR & LR(Cplex) & Opt \\\\\\ \n " >> result.txt



UseIntraCons=1

demand_type=3

for n in 20 ; do
  for T in 24 48 ; do
    for id in {1..10}; do
      for met in 200 ; do
        ./bin/SCIP_UCP_BP.linux.x86_64.gnu.opt.cpx 1 $dossier $n $T 1 $demand_type $sym $cat01 $intra $id $met $UseIntraCons
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




printf "NO INTRA SITE CONSTRAINTS" >> result.txt
printf "\n" >> result.txt

UseIntraCons=0

demand_type=3

for n in 20 ; do
  for T in 24 ; do
    for id in {1..10} ; do
      for met in 102 1022 ; do
        ./bin/SCIP_UCP_BP.linux.x86_64.gnu.opt.cpx 1 $dossier $n $T 1 $demand_type $sym $cat01 $intra $id $met $UseIntraCons
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

