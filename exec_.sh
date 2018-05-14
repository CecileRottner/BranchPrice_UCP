#!/bin/bash
rm result.txt

dossier=data/sym=site/

nom=result.txt

n=20
T=24
sym=3
demand_type=4

  for id in {7..20}; do
      ./bin/SCIP_UCP_BP.linux.x86_64.gnu.opt.cpx 1 $dossier $n $T 1 $demand_type $sym 0 1 $id $nom
  done

printf "\n" >> result.txt
printf "\n" >> result.txt



