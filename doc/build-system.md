# The Bara Build System

A lightweight dependency-based build tool for C projects. Bara scans local include directives, recursively compiles dependent source files, auto-generates header files via a Python prototype extractor, and links binaries—all with minimal configuration.

---

## Features

- **Recursive Dependency Resolution:**
  Scans local `#include "..."` directives; if an included header has a corresponding `.c` source, it’s processed automatically.

- **Auto Header Generation:**
  Invokes `tools/extract_prototypes.py` to generate header files with standardized include guards derived from file paths.

- **Incremental Builds:**
  Timestamp checks avoid unnecessary recompilation and linking.

- **Customizable Build Flags:**
  Override default `CFLAGS` (`-O2 -Wall -Wextra`) and `LDFLAGS` (`-ldl`) via environment variables.

---

## Directory Layout

```
project-root/
├── src/
│   └── programs/         # C program files (entry points)
│   └── lib/         	  # Example for source library
├── bin/                  # Output binaries
│   └── tmp/              # Intermediate object files
├── tools/
│   └── extract_prototypes.py  # Python header extraction tool
└── build.sh              # Bara build script
```

---

## Usage

1. **Setup:**
   Prerequisites:
   - Bash shell
   - GCC
   - Python3 (for header generation)

2. **Project Organization:**
   - Place main C programs in `src/programs/`.
   - Maintain related dependency files alongside or referenced via local includes.

3. **Run the Build:**
   From the project root, execute:
   ```bash
   ./build.sh
   ```
   This will build every program in src/programs/ and generate an equally named executable.
   Bara compiles modified sources, generates headers if needed, and links the final executable(s) into `bin/`.

4. **Custom Flags:**
   Customize compilation/linking by setting environment variables:
   ```bash
   CFLAGS="-O3 -march=native" LDFLAGS="-ldl -lm" ./build.sh
   ```

---

## How It Works

- **Header Generation:**
  For each `.c` file, if the corresponding header is missing or outdated, Bara runs:
  ```bash
  python3 tools/extract_prototypes.py <source.c>
  ```
  The output is wrapped with include guards computed from the file’s relative path.

- **Dependency Processing:**
  `process_includes()` recursively resolves local header dependencies, compiling each related `.c` file (if present) into `bin/tmp/`.

- **Linking:**
  Main sources in `src/programs/` are compiled and linked with their dependencies. Timestamp comparisons ensure only modified files trigger rebuilds.

---

## Cleaning Up

To remove build artifacts:
```bash
rm -rf bin
```

---
