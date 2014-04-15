#!/bin/bash

GOV=${1:-ondemand}
FRQ=${2:-700000}

echo ".:: CPUFreq Support"
echo -n "Available governors: "
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors
echo -n "Available frequencies: "
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies
echo

echo ".:: CPUFreq Configuration"
echo -n "Select [$GOV] governor"
for i in `seq 0 256`; do
	[ -f /sys/devices/system/cpu/cpu$i/cpufreq/scaling_governor ] || break
	echo $GOV > /sys/devices/system/cpu/cpu$i/cpufreq/scaling_governor
	echo -n "."
done
echo
if [ $GOV == "userspace" ]; then
  echo -n "Select [$FRQ] frequency"
  for i in `seq 0 256`; do
    [ -f /sys/devices/system/cpu/cpu$i/cpufreq/scaling_governor ] || break
    echo $FRQ > /sys/devices/system/cpu/cpu$i/cpufreq/scaling_setspeed
    echo -n "."
  done
  echo
fi
echo

echo ".:: Current CPUs Configuration"
echo -n "Governor: "
cat /sys/devices/system/cpu/cpu?/cpufreq/scaling_governor | sort -u
if [ $GOV != "ondemand" ]; then
  echo -n "Frequency: "
  cat /sys/devices/system/cpu/cpu?/cpufreq/scaling_cur_freq | sort -u
fi
echo

