#!/usr/bin/env bash

rm -Rf *.ldr

make clean
make PLATFORM=LTE1 FLASH_TYPE=intel16 clean
make PLATFORM=LTE2 FLASH_TYPE=intel16 clean
make PLATFORM=LTE1C FLASH_TYPE=intel16 clean
make PLATFORM=LTE2C FLASH_TYPE=intel16 clean
make PLATFORM=LTE1 FLASH_TYPE=amd16 clean

make PLATFORM=LTE1 FLASH_TYPE=intel16
mv hitagi.ldr Hitagi_LTE1_Intel_16.ldr
make PLATFORM=LTE1 FLASH_TYPE=intel16 clean

make PLATFORM=LTE2 FLASH_TYPE=intel16
mv hitagi.ldr Hitagi_LTE2_Intel_16.ldr
make PLATFORM=LTE2 FLASH_TYPE=intel16 clean

make PLATFORM=LTE1C FLASH_TYPE=intel16
mv hitagi.ldr Hitagi_LTE1_Compact_Intel_16.ldr
make PLATFORM=LTE1C FLASH_TYPE=intel16 clean

make PLATFORM=LTE2C FLASH_TYPE=intel16
mv hitagi.ldr Hitagi_LTE2_Compact_Intel_16.ldr
make PLATFORM=LTE2C FLASH_TYPE=intel16 clean

make PLATFORM=LTE1 FLASH_TYPE=amd16
mv hitagi.ldr Hitagi_LTE1_AMD_16.ldr
make PLATFORM=LTE1 FLASH_TYPE=amd16 clean
