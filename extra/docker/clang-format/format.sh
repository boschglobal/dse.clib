#!/bin/bash

target=$1;
clang_cmd="clang-format";
clang_opts="--Wno-error=unknown -i  --verbose";

# Determine if a Clang Format rule file is needed.
if [ -f $target ]; then
	echo "Target is $target (file).";
	path=$(dirname "$(realpath $target)");
else
	echo "Target is $target (directory).";
	path=$target;
fi
if [ ! -f "$path/.clang-format" ]; then
	echo "Using DSE Clang Format rules file.";
	cp /usr/local/etc/clang-format .clang-format
	clang_cleanup=1;
fi

# Run Clang Format.
if [ -f $target ]; then
	$clang_cmd $clang_opts $target;
else
	echo "$(find $target -type d -name build -prune -o \( -iname *.h -o -iname *.c \) -print | xargs $clang_cmd $clang_opts)";
fi
# Cleanup, if necessary.
if [ ! -z "$clang_cleanup" ]; then
	rm -f .clang-format
fi
