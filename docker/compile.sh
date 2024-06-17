#!/bin/bash
# ./entry.sh compile /home/gurgui/results

############## Variables #################
args=($@)								 	
outpath="${args[0]}"
cdate=$(date '+%d/%m/%Y @ %H:%M')
msg="$cdate -"
debug=false
osname=$(awk -F= '$1=="PRETTY_NAME" {print $2}' /etc/os-release)
tmpdir=$(mktemp -d)
tmpfile="$tmpdir/.msg"

declare -A checks
checkDescriptions=("Create new profile" "Build certificate authority" "Build server certifficate" "Build client certificate")
checkCmds=("init -n foo -s $tmpdir/foo" "build foo ca -cn fooCA -y" "build foo sv -cn fooSV -y" "build foo cl -cn fooCL -y")
##########################################

for i in ${args[@]}; do
	if [[ "${i,,}" == "-d" || "${i,,}" == --debug ]]; then
		debug=true
	fi
done

if (( $(id -u) != 2222 )); then
	$debug && printf "wrong id '%i'\n" "$(id -u)"
	exit 1
fi

if [ -z "$outpath" ]; then
	$debug && printf "outpath is|are empty\n"
	exit 1
fi

if [ ! -w "$outpath" ]; then
	$debug && printf "outpath '%s' is not writable\n" "$outpath"
	exit 1
fi

cd "/home/gpkih" || exit 1
cp -rf src config CMakeLists.txt setup "$tmpdir" 	
cd "$tmpdir"
echo -e "System: $osname\nDate: $cdate" > "$tmpfile"

./setup -q

printf "Compilation status: " >> "$tmpfile"
(( $? == 0 )) && printf "success\n" >> "$tmpfile" || printf "failure\n" >> "$tmpfile"

# Do extra checks
echo -e "  [+] Running checks" >> "$tmpfile"
for (( i=0 ; i < "${#checkCmds[@]}" ; ++i )); do
	description="${checkDescriptions[$i]}"
	cmd="${checkCmds[$i]}"
	./gpkih $cmd &>/dev/null
	printf "  Description: %s\n    cmd: %s\n    exit code: %i\n" "$description" "$cmd" "$?" >> "$tmpfile"
done

# Add results to result file
cat "$tmpfile" >> "$outpath"