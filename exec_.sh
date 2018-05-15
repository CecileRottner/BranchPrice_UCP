#!/bin/bash
rm result.txt

nom=result.txt

n=30
sym=4
demand_type=4


dossier=data/sym=site/

for T in 24 48 96 ; do
  for id in {1..20}; do
      ./bin/SCIP_UCP_BP.linux.x86_64.gnu.opt.cpx 1 $dossier $n $T 1 $demand_type $sym 0 1 $id $nom
  done

printf "\n" >> result.txt
printf "\n" >> result.txt


