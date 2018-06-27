#!/bin/bash
rm result.txt

nom=result.txt

n=3
T=5
sym=3
demand_type=3
cat01=0
intra=1


n=20
T=48
sym=0
demand_type=3
cat01=1
intra=0

dossier=data/Pmin=Pmax/

for id in {1..20}; do
  for met in 0 ; do
      ./bin/SCIP_UCP_BP.linux.x86_64.gnu.opt.cpx 1 $dossier $n $T 1 $demand_type $sym $cat01 $intra $id $met
  done
done

printf "\n" >> result.txt
printf "\n" >> result.txt










