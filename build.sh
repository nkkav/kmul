#!/bin/bash

rm -rf kmul*.nac kmulu*.c kmuls*.c
make clean
make

if [ "$SECONDS" -eq 1 ]
then
  units=second
else
  units=seconds
fi
echo "This script has been running for $SECONDS $units."
exit 0
