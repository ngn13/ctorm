#!/bin/bash

# docker init script

examples=()
pids=()

for example in dist/*; do
  examples+=("${example}")
done

for i in "${!examples[@]}"; do
  "./${examples[$i]}" &
  pids[$i]=$!
done

for i in "${!pids[@]}"; do
  if ! wait "${pids[$i]}"; then
    echo "${examples[$i]} exited with a non zero exit-code (${?})"
    exit 1
  fi
done

exit 0
