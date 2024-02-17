#!/bin/bash

if (( "$EUID" != 0 )); then
	printf "need sudo privileges\n"
	exit 0
fi

script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
build_dir="$script_dir/src/build"
cmakelists="$script_dir/src/CMakeLists.txt"

args=($@)
argc=$#

if [ -e "$build_dir" ]; then
	rm -rf "$build_dir"	
fi

mkdir "$build_dir"

cd "$build_dir"

_commands=("cmake .." "make" "cp gpkih ../gpkih")

for command in $_commands; do
	${command[@]}
	if(( $? != 0 )); then
		exit $?
	fi 
done