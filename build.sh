#!/bin/bash
cd "$(dirname "$0")"

OPT_FLAG=${1:-"O0"}

echo "Build mode: $OPT_FLAG"

mkdir bin > /dev/null 2>&1
mkdir bin/tmp > /dev/null 2>&1

COMPILER="gcc"
FLAGS="-w -Wall -$OPT_FLAG"

time -p $COMPILER compiler/capy.c $FLAGS $LIBS -o bin/capy.$OPT_FLAG.linux.bin

if [ $? -eq 0 ]
then
	ls -lh bin/
	exit 0
else
	exit 1
fi
