#!/bin/bash

EXE=.exe

HILIMIT1=255
HILIMIT2=127
LOLIMIT2=-128
#HILIMIT1=65535
#HILIMIT2=32767
#LOLIMIT2=-32768

# Test unsigned multiplications
for ((mulu=0 ; mulu <= ${HILIMIT1} ; mulu++))
do
  echo ""
  echo "Optimizing ${mulu} * x"
  ./kmul${EXE} -d -mul ${mulu} -width 32 -unsigned -nac
  ./kmul${EXE} -d -mul ${mulu} -width 32 -unsigned -ansic
done

# Test signed multiplications
for ((muls=${LOLIMIT2} ; muls <= ${HILIMIT2} ; muls++))
do
  echo ""
  echo "Optimizing ${muls} * x"
  ./kmul${EXE} -d -mul ${muls} -width 32 -signed -nac
  ./kmul${EXE} -d -mul ${muls} -width 32 -signed -ansic
done

if [ "$SECONDS" -eq 1 ]
then
  units=second
else
  units=seconds
fi
echo "This script has been running for $SECONDS $units."

