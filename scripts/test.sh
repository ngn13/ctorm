#!/bin/bash

export LD_LIBRARY_PATH='./dist'
examples=(
  "hello"
  "echo"
  "params"
  "locals"
  "middleware"
)
index="${1}"

function run_example(){
  echo "${1}: testing..."

  ./dist/example_${1} &
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

  run_example "${examples[${index}]}"
  exit 0
fi

for example in "${examples[@]}"; do
  if ! run_example "${example}"; then
    exit 1
  fi
done
