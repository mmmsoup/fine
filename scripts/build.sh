#!/bin/sh

# thank you https://stackoverflow.com/questions/4774054/reliable-way-for-a-bash-script-to-get-the-full-path-to-itself
SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

BASEPATH="$SCRIPTPATH/.."
BUILDPATH="$(realpath "$BASEPATH/build")"

[[ $1 == "clean" ]] && rm -r "$BUILDPATH"

if stat "$BUILDPATH/Makefile" &> /dev/null; then
	cd "$BUILDPATH"
	make
else
	mkdir -p "$BUILDPATH"
	cd "$BUILDPATH"
	cmake ..
	make
fi
