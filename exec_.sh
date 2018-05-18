#!/bin/bash
rm result.txt

nom=result.txt

n=10
sym=0
demand_type=3


dossier=data/

for T in 24 ; do
  for id in {2..20}; do
      ./bin/SCIP_UCP_BP.linux.x86_64.gnu.opt.cpx 1 $dossier $n $T 1 $demand_type $sym 0 1 $id $nom
  done
  printf "\n" >> result.txt
  printf "\n" >> result.txt
done

printf "\n" >> result.txt
printf "\n" >> result.txt

