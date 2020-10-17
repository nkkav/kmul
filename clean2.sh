#!/bin/bash

#absval=

# Advanced Bash Scripting Guide, p. 699
abs ()
{
  if [ "$1" -lt 0 ]
  then
    let "absval = 0 - $1"
  else
    let "absval = $1"
  fi
}

HILIMIT1=255
HILIMIT2=127
LOLIMIT2=-128
#HILIMIT1=65535
#HILIMIT2=32767
#LOLIMIT2=-32768

# Clean the produced files from testing unsigned multiplications
for ((mulu=0 ; mulu <= ${HILIMIT1} ; mulu++))
do
  for alg in "o"; do
    rm -rf kmul_${alg}_u32_p_${mulu}.nac 
    rm -rf kmul_${alg}_u32_p_${mulu}.c
  done
done

# Clean the produced files from testing signed multiplications
for ((muls=${LOLIMIT2} ; muls <= ${HILIMIT2} ; muls++))
do
  for alg in "o"; do
    abs $muls
    rm -rf kmul_${alg}_s32_m_${absval}.nac
    rm -rf kmul_${alg}_s32_m_${absval}.c
    rm -rf kmul_${alg}_s32_p_${absval}.nac 
    rm -rf kmul_${alg}_s32_p_${absval}.c
  done
done

if [ "$SECONDS" -eq 1 ]
then
  units=second
else
  units=seconds
fi
echo "This script has been running for $SECONDS $units."
