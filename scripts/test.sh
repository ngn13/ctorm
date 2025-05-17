#!/bin/bash

export LD_LIBRARY_PATH='./dist'
examples=(
  "hello"
  "echo"
  "params"
  "locals"
  "middleware"
  "multithread"
)
index="${1}"

function run_example(){
  echo "${1}: testing..."

  ./dist/example_${1} &
  sleep 1

  ./scripts/test_${1}.sh
  res=$?

  kill -9 $!

  if [ $res -ne 0 ]; then
    echo "${1}: failed"
    return 1
  fi

  echo "${1}: success"
  return 0
}

if [ ! -z "${index}" ]; then
  index="$((index-1))"

  if [ ! "${index}" -lt "${#examples[@]}" ] || [ "${index}" -lt 0 ]; then
    echo "invalid example number: ${1}"
    exit 1
  fi

  run_example "${examples[${index}]}" && exit 0
  exit 1
fi

for example in "${examples[@]}"; do
  run_example "${example}" || exit 1
done
