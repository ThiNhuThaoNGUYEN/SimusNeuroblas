#! /bin/bash

################################################################################
# $1 the "bin" directory where one shall find e.g. the simuscale executable
# $2 a specific test's directory
################################################################################

cd $2
rm result/*.txt result/backup_*
$1/simuscale -i ref -o result

fail=0
if ! diff -q ref/normalization.txt result/normalization.txt; then fail=1; fi
if ! diff -q ref/trajectory.txt result/trajectory.txt; then fail=1; fi

for file in result/backup_*; do
  if ! diff -q ref/$(basename $file) result/$(basename $file); then fail=1; fi
done

if test $fail -ne 0; then exit -1; fi

