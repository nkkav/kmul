#!/bin/bash

# Clean the produced files from testing unsigned and signed multiplications
for mulu in "0" "1" "2" "3" "4" "5" "6" "7" "8" "9" "10" "11" "23" "37" "43" "111" "255"
do
  for alg in "b" "o"; do
    rm -rf kmul_${alg}_u32_p_${mulu}.nac 
    rm -rf kmul_${alg}_u32_p_${mulu}.c
  done
done

for muls in "1" "2" "3" "4" "5" "6" "7" "8" "9" "10" "11" "23" "37" "43" "111" "255"
do
  rm -rf kmul_${alg}_s32_m_${muls}.nac 
  rm -rf kmul_${alg}_s32_m_${muls}.c
  rm -rf kmul_${alg}_s32_p_${muls}.nac 
  rm -rf kmul_${alg}_s32_p_${muls}.c
done
