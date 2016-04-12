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


# Clean the produced files from testing unsigned and signed multiplications
for mulu in "0" "1" "2" "3" "4" "5" "6" "7" "8" "9" "10" "11" "23" "37" "43" "111" "255"
do
  rm -rf kmul_u32_p_${mulu}.nac 
  rm -rf kmul_u32_p_${mulu}.c
done
for muls in "-255" "-111" "-43" "-3" "-2" "-1" "1" "2" "3" "4" "5" "6" "7" "8" "9" "10" "11" "23" "37" "43" "111" "255"
do
  $(abs $muls)
  rm -rf kmul_s32_m_${absval}.nac 
  rm -rf kmul_s32_m_${absval}.c
  rm -rf kmul_s32_p_${muls}.nac 
  rm -rf kmul_s32_p_${muls}.c
done

exit 0
