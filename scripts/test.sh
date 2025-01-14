#!/bin/bash

set -e

export LD_LIBRARY_PATH='./dist'
examples=("hello" "echo" "params" "middleware")
index="${1}"

function run_example(){
  echo "testing example: ${1}"
  (./dist/example_${1} & 2>/dev/null)
  ./scripts/test_${1}.sh
  killall -9 "example_${1}"
}

if [ ! -z "${index}" ]; then
  index="$((index-1))"

  if [ ! "${index}" -lt "${#examples[@]}" ] || [ "${index}" -lt 0 ]; then
    echo "invalid example number: ${1}"
    exit 1
  fi

  run_example "${examples[${index}]}"
  exit 0
fi

for example in "${examples[@]}"; do
  run_example "${example}"
done
