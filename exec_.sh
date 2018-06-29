#!/bin/bash
rm result.txt

nom=result.txt

sym=0
demand_type=3
cat01=1
intra=0

dossier=data/Pmin=Pmax/

printf "met & n & T & id & nodes & IUP & Iter & Var & CPU & gap & RL & low & up & cplex heur \\\\\\ \n " >> result.txt

for n in 10 20 ; do
  for T in 24 48 ; do
    for id in {1..20}; do
      for met in -1 1 2 3 4 ; do
        ./bin/SCIP_UCP_BP.linux.x86_64.gnu.opt.cpx 1 $dossier $n $T 1 $demand_type $sym $cat01 $intra $id $met
      done
      printf "\n" >> result.txt
    done
   printf "\n" >> result.txt
  done
  printf "\n" >> result.txt
done
printf "\n" >> result.txt
printf "\n" >> result.txt






