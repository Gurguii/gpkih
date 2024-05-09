#!/bin/bash
# bash ./entry.sh compile /home/gurgui/results

############## Variables #################
args=($@)								 	
action="${args[0]}"
outpath="${args[1]}"
cdate=$(date '+%d_%m_%Y @ %H:%M')
msg="$cdate -"
debug=false

osname=$(awk -F= '$1=="PRETTY_NAME" {print $2}' /etc/os-release)
##########################################

for i in ${args[@]}; do
	if [[ "${i,,}" == "-d" || "${i,,}" == --debug ]]; then
		debug=true
	fi
done

if [[ -z "$action" || -z "$outpath" ]]; then
	$debug && printf "action|outpath is|are empty\n"
	exit 1
fi

if [[ ! -w "$outpath" ]]; then
	$debug && printf "outpath '%s' is not writable\n" "$outpath"
	exit 1
fi

cd "/home/gpkih" || exit 1

if [[ "$action" == "compile" ]]; then
	tmpdir=$(mktemp -d)
	tmpfile="$tmpdir/.msg"
	extra_checks=("init -n foo -s $tmpdir/foo" "build foo ca -cn fooCA" "build foo sv -cn fooSV -y" "reset")
	required_to_build="src config CMakeLists.txt setup"
	
	cp -rf $required_to_build "$tmpdir" 	
	cd "$tmpdir"

	echo -e "Operating System - $osname\nDate: $cdate" > "$tmpfile"
	./setup -q

	if (( $? == 0 )); then
		# Success compiling
		echo -e "\tRESULT - success - compile went well" >> "$tmpfile"
	else
		# Fail compiling
		echo -e "\tRESULT - failure - compile went wrong" >> "$tmpfile"
	fi

	# Do extra checks if required
	if (( ${#extra_checks[@]} > 0 )); then 
		for check in "${extra_checks[@]}"; do
			command="./gpkih $check"
			$command &>/dev/null
			echo -e "\t\tCHECK $command returned [$?]" >> "$tmpfile"
		done
	fi

	# Add results to result file
	cat "$tmpfile" >> "$outpath"
fi