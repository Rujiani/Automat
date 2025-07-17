#!/usr/bin/env bash
exe=$1
in=$2
ref=$3
maze_file=${4:-$(dirname "$in")/maze_default.txt}

out=$(mktemp)
"$exe" "$maze_file" < "$in" > "$out"
diff -u "$ref" "$out"
