#!/bin/bash
rm result.txt

nom=result.txt


n=20
sym=0
demand_type=3
cat01=1
intra=0

dossier=data/Pmin=Pmax/

for T in 24 ; do
  for id in {1..20}; do
      ./bin/SCIP_UCP_BP.linux.x86_64.gnu.opt.cpx 1 $dossier $n $T 1 $demand_type $sym $cat01 $intra $id $nom
  done
  printf "\n" >> result.txt
  printf "\n" >> result.txt
done

printf "\n" >> result.txt
printf "\n" >> result.txt







