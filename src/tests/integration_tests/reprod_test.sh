#! /bin/bash

################################################################################
# $1 the "bin" directory where one shall find e.g. the simuscale executable
# $2 a specific test's directory
################################################################################

cd $2
rm -rf first second
mkdir first second
$1/simuscale -o first
$1/simuscale -r 5.0 -i first -o second

fail=0
if ! diff -q second/normalization.txt first/normalization.txt; then fail=1; fi
if ! diff -q second/trajectory.txt first/trajectory.txt; then fail=1; fi

for file in second/backup_*; do
  if ! diff -q second/$(basename $file) first/$(basename $file); then fail=1; fi
done

if test $fail -ne 0; then exit -1; fi
