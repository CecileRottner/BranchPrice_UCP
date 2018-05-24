#!/bin/bash
rm result.txt

nom=result.txt

n=20
sym=3
demand_type=4
cat01=0


dossier=data/sym=site/

for T in 24 ; do
  for id in {1..20}; do
      ./bin/SCIP_UCP_BP.linux.x86_64.gnu.opt.cpx 1 $dossier $n $T 1 $demand_type $sym $cat01 1 $id $nom
  done
  printf "\n" >> result.txt
  printf "\n" >> result.txt
done

printf "\n" >> result.txt
printf "\n" >> result.txt

n=30
sym=4

for T in 24 ; do
  for id in {1..20}; do
      ./bin/SCIP_UCP_BP.linux.x86_64.gnu.opt.cpx 1 $dossier $n $T 1 $demand_type $sym $cat01 1 $id $nom
  done
  printf "\n" >> result.txt
  printf "\n" >> result.txt
done

printf "\n" >> result.txt
printf "\n" >> result.txt


dossier=data/sym_C_site/

n=20
sym=3

for T in 24 ; do
  for id in {1..20}; do
      ./bin/SCIP_UCP_BP.linux.x86_64.gnu.opt.cpx 1 $dossier $n $T 1 $demand_type $sym $cat01 1 $id $nom
  done
  printf "\n" >> result.txt
  printf "\n" >> result.txt
done

printf "\n" >> result.txt
printf "\n" >> result.txt
