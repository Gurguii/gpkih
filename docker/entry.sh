#!/bin/bash
# bash ./entry.sh compile /home/gurgui/results

if (( $(id -u) != 2222 )); then
	exit 1
fi

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
	
	extra_checks=("init -n foo -s $tmpdir/foo" "build foo ca -cn fooCA" "build foo sv -cn fooSV -y" "build foo cl -cn fooCL -y")

	required_to_build="src config CMakeLists.txt setup"
	
	cp -rf $required_to_build "$tmpdir" 	
	cd "$tmpdir"

	echo -e "System: $osname\nDate: $cdate" > "$tmpfile"

	./setup -q

	if (( $? == 0 )); then
		# Success compiling
		echo -e "Status: success" >> "$tmpfile"
	else
		# Fail compiling
		echo -e "Status: fail" >> "$tmpfile"
	fi

	# Do extra checks if required
	if (( ${#extra_checks[@]} > 0 )); then
		echo -e "  [+] Running checks" >> "$tmpfile" 
		for command in "${extra_checks[@]}"; do
			./gpkih $command &>/dev/null
			echo -e "      [command]: $command\n      [exit-code]: $?" >> "$tmpfile"
		done
	fi

	# Add results to result file
	cat "$tmpfile" >> "$outpath"
fi