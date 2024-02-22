#!/bin/bash

if (( "$EUID" == 0 )); then
	printf "not running with elevated privileges\n"
	exit 0
fi

base_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
src_dir="$base_dir/src"
build_dir="$base_dir/build"
binary_outpath="$base_dir/gpkih"

cmakelists="$base_dir/CMakeLists.txt"

log_stderr="$base_dir/.error.log"

if [ -e "$log_stderr" ]; then
	rm "$log_stderr"
fi

args=($@)
argc=$#

if ! [ -e "$build_dir" ]; then
	mkdir "$build_dir"	
	if (( $? != 0 )); then
		printf "couldn't create build dir '%s'\n" "$build_dir"
		exit 1
	fi
fi

cd "$build_dir"

declare -A subopts
subopts["--create-packs"]="make package"
subopts["--create-source-packs"]="make package_source"

# Commands to run in order to build the executable
# note: this will be executed inside build/ 
commands=("cmake .." "make -j" "cp gpkih $binary_outpath")

for arg in "${args[@]}"; do
	if [ "$arg" == "-h" ]; then
		cat << EOF

Usage: setup.sh [optional-subopts]

[optional-subopts]
  --create-packs         : adds 'make package' to the command list
  --create-source-packs  : adds 'make package_source' to the command list

EOF
	exit 0
fi
	if [ -n "${subopts[$arg]}" ]; then
		commands+=("${subopts[$arg]}")
	fi
done

for command in "${commands[@]}"; do
	printf "[running] %s\n" "$command" 
	$command 2>> "$log_stderr"
	if(( $? != 0 )); then
		printf "Something failed running '%s', check '%s' for more information" "$command" "$log_stderr"
		exit $?
	fi 
done

printf "[info] gpkih succesfully compiled: %s\n" "$binary_outpath"