#!/bin/bash
set -e

# Directories.
SRC_LIB="src/lib"
SRC_PROG="src/programs"
BIN_DIR="bin"
TMP_DIR="$BIN_DIR/tmp"
PROJECT_ROOT="$(pwd)"  # Used for generating unique object names.

# Build flags (override via env vars)
CFLAGS="${CFLAGS:--O2 -Wall -Wextra}"
LDFLAGS="${LDFLAGS:-} -ldl"

# Colors.
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
BLUE='\033[1;94m'
GRAY='\033[90m'
NC='\033[0m'

mkdir -p "$BIN_DIR" "$TMP_DIR"

# generate_header():
#   Uses the tool (tools/extract_prototypes.py) to generate a header from a C file.
generate_header() {
    local src="$1"
    local h_file="${src%.c}.h"
    local rel_path
    rel_path=$(echo "$h_file" | sed "s|^$PROJECT_ROOT/||")
    local guard
    guard=$(echo "$rel_path" | tr '[:lower:]' '[:upper:]' | sed 's/[^A-Z0-9]/_/g')

    echo -e "${YELLOW}[BUILD]${NC} Generating header: ${GREEN}$h_file${NC}"
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
}

# get_obj_path():
#   Generate a unique object file name based on the relative path to PROJECT_ROOT.
get_obj_path() {
    local src_file="$1"
    local rel
    rel=$(realpath --relative-to="$PROJECT_ROOT" "$src_file")
    # Replace directory separators with underscores.
    local obj_name
    obj_name=$(echo "$rel" | sed 's/[\/]/_/g')
    echo "$TMP_DIR/${obj_name%.c}.o"
}

###########################################
# 1. Generate headers for library sources
###########################################
echo -e "${YELLOW}[BUILD]${NC} Generating headers for library sources from ${GREEN}$SRC_LIB${NC}..."
while IFS= read -r -d '' src; do
    local_header="${src%.c}.h"
    if [ ! -f "$local_header" ] || [ "$src" -nt "$local_header" ]; then
        generate_header "$src"
    fi
done < <(find "$SRC_LIB" -type f -name "*.c" -print0)

###########################################
# 2. Compile library objects
###########################################
echo -e "${YELLOW}[BUILD]${NC} Compiling library sources from ${GREEN}$SRC_LIB${NC}..."
lib_objs=""
while IFS= read -r -d '' src; do
    obj=$(get_obj_path "$src")
    if [ ! -f "$obj" ] || [ "$src" -nt "$obj" ]; then
        echo -e "${YELLOW}[BUILD]${NC} Compiling lib: ${GREEN}$src${NC} -> ${BLUE}$obj${NC}"
        gcc $CFLAGS -c "$src" -o "$obj"
    fi
    lib_objs="$lib_objs $obj"
done < <(find "$SRC_LIB" -type f -name "*.c" -print0)

###########################################
# 3. Build programs
###########################################
# processed: associative array to track dependency files.
declare -A processed

# process_includes():
#   Recursively scan a file for local (quoted) includes.
process_includes() {
    local file
    file=$(realpath "$1")
    if [ -n "${processed[$file]}" ]; then
        return
    fi
    processed["$file"]=1
    local dir
    dir=$(dirname "$file")

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
}

echo -e "${YELLOW}[BUILD]${NC} Building program sources from ${GREEN}$SRC_PROG${NC}..."
# Compute absolute path for SRC_LIB.
lib_real=$(realpath "$SRC_LIB")
for main_src in "$SRC_PROG"/*.c; do
    [ -e "$main_src" ] || continue
    main_src=$(realpath "$main_src")
    # Reset processed dependencies for each program.
    processed=()
    process_includes "$main_src"

    prog_objs=""
    # Gather dependency objects (only for .c files and excluding main_src).
    for file in "${!processed[@]}"; do
        if [[ "$file" =~ \.c$ ]]; then
            # Skip files in the lib directory as they are already built.
            if [[ "$file" != "$main_src" && "$file" != "$lib_real"* ]]; then
                dep_obj=$(get_obj_path "$file")
                prog_objs="$prog_objs $dep_obj"
                # Compile dependency if needed.
                if [ ! -f "$dep_obj" ] || [ "$file" -nt "$dep_obj" ]; then
                    echo -e "${YELLOW}[BUILD]${NC} Compiling program dependency: ${GREEN}$file${NC} -> ${BLUE}$dep_obj${NC}"
                    gcc $CFLAGS -c "$file" -o "$dep_obj"
                fi
            fi
        fi
    done

    # Compile the main program file.
    main_obj=$(get_obj_path "$main_src")
    if [ ! -f "$main_obj" ] || [ "$main_src" -nt "$main_obj" ]; then
        echo -e "${YELLOW}[BUILD]${NC} Compiling program main: ${GREEN}$main_src${NC} -> ${BLUE}$main_obj${NC}"
        gcc $CFLAGS -c "$main_src" -o "$main_obj"
    fi

    output="$BIN_DIR/$(basename "$main_src" .c)"
    rebuild=0
    if [ ! -f "$output" ]; then
        rebuild=1
    elif [ "$main_src" -nt "$output" ]; then
        rebuild=1
    else
        for o in $prog_objs; do
            [ -f "$o" ] && [ "$o" -nt "$output" ] && { rebuild=1; break; }
        done
        for o in $lib_objs; do
            [ -f "$o" ] && [ "$o" -nt "$output" ] && { rebuild=1; break; }
        done
    fi

    if [ "$rebuild" -eq 1 ]; then
        echo -e "${YELLOW}[BUILD]${NC} Linking program: ${GREEN}$main_src${NC} -> ${BLUE}$output${NC}"
        gcc "$main_obj" $prog_objs $lib_objs $LDFLAGS -o "$output"
    else
        echo -e "${GRAY}[UNCHANGED]${NC} ${BLUE}$output${NC}"
    fi
done