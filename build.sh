#!/bin/bash
set -e

SRC_DIR="src/programs"
BIN_DIR="bin"
TMP_DIR="$BIN_DIR/tmp"
PROJECT_ROOT="$(pwd)"  # Root path to strip from include guards

# Build flags (override via env vars)
CFLAGS="${CFLAGS:--O2 -Wall -Wextra}"
LDFLAGS="${LDFLAGS:-} -ldl"

YELLOW='\033[1;33m'
GREEN='\033[0;32m'
BLUE='\033[1;94m'
GRAY='\033[90m'
NC='\033[0m'

mkdir -p "$BIN_DIR" "$TMP_DIR"

# generate_header():
#   Uses the Python tool (tools/extract_prototypes.py) to extract prototypes
#   from the source file, then wraps that output with include guards.
generate_header() {
    local src="$1"
    local base
    base=$(basename "$src" .c)
    local h_file="${src%.c}.h"
    # Compute a relative path from PROJECT_ROOT for a cleaner guard name.
    local rel_path
    rel_path=$(echo "$h_file" | sed "s|^$PROJECT_ROOT/||")
    local guard
    guard=$(echo "$rel_path" | tr '[:lower:]' '[:upper:]' | sed 's/[^A-Z0-9]/_/g')

    if [ ! -f "$h_file" ] || [ "$src" -nt "$h_file" ]; then
        echo -e "${YELLOW}[BUILD]${NC} Generating header: ${GREEN}$h_file${NC}"
        # Call the external Python tool to extract prototypes.
        python3 tools/extract_prototypes.py "$src" > "$h_file.tmp"
        {
            echo "/* Auto-generated header from $(basename "$src") */"
            echo "/* DO NOT EDIT MANUALLY */"
            echo "#ifndef $guard"
            echo "#define $guard"
            echo
            cat "$h_file.tmp"
            echo
            echo "#endif /* $guard */"
        } > "$h_file"
        rm "$h_file.tmp"
    fi
}

# processed tracks files already handled to avoid duplicate work.
declare -A processed

# process_includes():
#   Recursively processes a file's dependencies by scanning for local (quoted)
#   includes. If a corresponding .c file exists for an include, process that file.
#   After processing dependencies, if the file is a .c source, compile it and,
#   unless told otherwise, generate its header.
#   An optional second parameter (e.g. "NOHEADER") prevents header generation.
process_includes() {
    local file
    file=$(realpath "$1")
    [ -n "${processed[$file]}" ] && return
    processed["$file"]=1
    local do_not_generate_header="$2"

    local dir
    dir=$(dirname "$file")

    # Process only local (quoted) includes for dependency resolution.
    local includes
    includes=$(grep -E '^[[:space:]]*#include[[:space:]]+"[^"]+\.h"' "$file" | sed -E 's/^[[:space:]]*#include[[:space:]]+"([^"]+\.h)".*/\1/')
    for inc in $includes; do
        local header_path
        header_path=$(realpath -m "$dir/$inc")
        local header_dir
        header_dir=$(dirname "$header_path")
        local base_dep
        base_dep=$(basename "$header_path" .h)
        local dep_source="$header_dir/${base_dep}.c"
        if [ -f "$dep_source" ]; then
            process_includes "$dep_source"
        elif [ -f "$header_path" ]; then
            process_includes "$header_path"
        fi
    done

    if [[ "$file" =~ \.c$ ]]; then
        local base
        base=$(basename "$file" .c)
        local obj="$TMP_DIR/${base}.o"
        if [ ! -f "$obj" ] || [ "$file" -nt "$obj" ]; then
            echo -e "${YELLOW}[BUILD]${NC} Compiling: ${GREEN}$file${NC} -> ${BLUE}$obj${NC}"
            gcc $CFLAGS -c "$file" -o "$obj"
        fi
        if [ -z "$do_not_generate_header" ]; then
            generate_header "$file"
        fi
    fi
}

# Main build loop.
for main_src in "$SRC_DIR"/*.c; do
    [ -e "$main_src" ] || continue
    main_src=$(realpath "$main_src")
    processed=()

    # Process all dependencies for the main source. Pass "NOHEADER" so that
    # headers aren’t generated for dependencies that are not final targets.
    process_includes "$main_src" "NOHEADER"

    objs=""
    # Exclude the main source from the dependency list.
    for file in "${!processed[@]}"; do
        if [[ "$file" =~ \.c$ && "$file" != "$main_src" ]]; then
            objs="$objs $TMP_DIR/$(basename "$file" .c).o"
        fi
    done

    base=$(basename "$main_src" .c)
    output="$BIN_DIR/$base"

    rebuild=0
    if [ ! -f "$output" ]; then
        rebuild=1
    elif [ "$main_src" -nt "$output" ]; then
        rebuild=1
    else
        for o in $objs; do
            [ -f "$o" ] && [ "$o" -nt "$output" ] && { rebuild=1; break; }
        done
    fi

    if [ "$rebuild" -eq 1 ]; then
        echo -e "${YELLOW}[BUILD]${NC} Linking: ${GREEN}$main_src${NC} -> ${BLUE}$output${NC}"
        # Compile main_src separately (its object is not duplicated in $objs).
        gcc $CFLAGS -c "$main_src" -o "$TMP_DIR/$(basename "$main_src" .c).o"
        gcc "$TMP_DIR/$(basename "$main_src" .c).o" $objs $LDFLAGS -o "$output"
    else
        echo -e "${GRAY}[UNCHANGED]${NC} ${BLUE}$output${NC}"
    fi
done
