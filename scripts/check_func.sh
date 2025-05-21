#!/bin/bash

# looks for banned functions, inspired from curl's checksrc.pl

functions=(
  # not thread safe
  "asctime"
  "ctime"
  "gmtime"
  "localtime"

  # may allow overflow
  "gets"
  "strtok"
  "sprintf"
  "vsprintf"
  "strcat"
  "strncat"
)

reset="\e[0m"
bold="${reset}\e[1m"
red="\e[31m"
blue="\e[34m"
green="\e[32m"

total=0

for func in "${functions[@]}"; do
  matches=$(grep -nr "${func}(.*)" src) || continue

  count=$(echo "${matches}" | wc -l)
  total=$((total+count))

  while read line; do
    file="$(echo "${line}" | cut -d: -f1)"
    num="$(echo "${line}" | cut -d: -f2)"

    len=$((${#file}+${#num}+2))
    match="$(echo ${line:$len} | sed 's/^ *//g')"

    printf \
      "${bold}${red}%s${bold} in ${blue}%s${bold}:${green}%d${reset} => %s\n" \
      "${func}" "${file}" "${num}" "${match}"
  done < <(echo "${matches}")
done

[ $total -eq 0 ] &&
  printf "${bold}no banned functions found${reset}\n" &&
  exit 0

if [ $total -eq 1 ]; then
  printf "${bold}found a banned function${reset}\n"
else
  printf "${bold}found total of %d banned functions${reset}\n" "${total}"
fi

exit 1
