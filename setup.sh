#!/bin/bash

if (( "$EUID" != 0 )); then
	printf "need sudo privileges\n"
	exit 0
fi

base_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
src_dir="$base_dir/src"
build_dir="$base_dir/build"

cmakelists="$base_dir/CMakeLists.txt"

log_stderr="$base_dir/.error.log"
log_stdout="$base_dir/.out.log"

if [ -e "$log_stderr" ]; then
	rm "$log_stderr"
fi

if [ -e "$log_stdout" ]; then
	rm "$log_stdout"
fi

args=($@)
argc=$#

if [ -e "$build_dir" ]; then
	rm -rf "$build_dir"	
fi

mkdir "$build_dir"

cd "$build_dir"

printf "[info] starting gpkih setup script - %s\n" "$(date '+%d-%m-%Y at %H:%M')" | tee -a "$log_stdout"
commands=("cmake .." "make" "cp gpkih ../gpkih")

for command in "${commands[@]}"; do
	printf "[running] %s\n" "$command" 
	$command 1>> "$log_stdout" 2>> "$log_stderr"
	if(( $? != 0 )); then
		printf "Something failed running '%s', check '%s' for more information" "$command" "$log_stderr"
		exit $?
	fi 
done

printf "[info] gpkih succesfully compiled: %s\n" "$src_dir/src/gpkih"
printf "[info] installation logs can be found at %s\n" "$log_stdout"
printf "[info] end of gpkih execution\n" | tee -a "$log_stdout" 