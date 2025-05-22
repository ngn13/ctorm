#!/bin/bash

# tag the current version

macro="$(grep "CTORM_VERSION" inc/ctorm.h)"
version="$(echo "${macro}" | awk '{print $3}' | sed 's/"//g')"

[ -z "${version}" ] && echo "failed to extract the version" && exit 1

echo "tagging version ${version}"
git tag "${version}"
