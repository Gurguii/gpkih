#!/bin/sh
# ./entry.sh compile /home/gurgui/ubuntu_results.txt

if [[ $EUID != 0 ]]; then
	exit 0
fi

# Variables #
args=($@)
action="${args[0]}"
outpath="${args[1]}"
cdate=$(date '+%d_%m_%Y @ %H:%M')
msg="$cdate -"
#############

if [[ -z "$action" || -z "$outpath" ]]; then
	exit 1
fi

cd "/home/gpkih" || exit 1

if [[ "$action" == "compile" ]]; then
	tmpdir=$(mktemp -d)
	extra_checks=("init -n foo -s $tmpdir/foo" "build foo ca -cn fooCA" "build foo sv -cn fooSV -y" "reset")
	required_to_build="src config CMakeLists.txt setup"
	cp -rf $required_to_build "$tmpdir" 	
	cd "$tmpdir"

	./setup -q

	if (( $? == 0 )); then
		# Success compiling
		echo "[ result ] success - $cdate - compile went well" > "$outpath"
	else
		# Fail compiling
		echo "[ result ] failure - $cdate - compile went wrong" > "$outpath"
	fi

	# Do extra checks if required
	if (( ${#extra_checks[@]} > 0 )); then 
		for check in "${extra_checks[@]}"; do
			command="./gpkih $check"
			$command &>/dev/null
			echo "[ check ] $command returned [$?]" >> "$outpath"
		done
	fi

	# Delete temporary directory
	rm -rf "$tmpdir" 
fi