#!/bin/bash

absval=

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
  rm -rf kmul_u32_p_${mulu}.nac 
  rm -rf kmul_u32_p_${mulu}.c
done

# Clean the produced files from testing signed multiplications
for ((muls=${LOLIMIT2} ; muls <= ${HILIMIT2} ; muls++))
do
  $(abs $muls)
  rm -rf kmul_s32_m_${absval}.nac 
  rm -rf kmul_s32_m_${absval}.c
done

exit 0

