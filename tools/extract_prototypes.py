#!/usr/bin/env python3
import sys, re

def remove_comments(code):
    # Remove C-style comments (/* … */) and C++-style comments (// …)
    code = re.sub(r'/\*.*?\*/', '', code, flags=re.DOTALL)
    code = re.sub(r'//.*', '', code)
    return code

def normalize_code(code):
    # Collapse spaces and tabs only, but keep newline characters intact.
    code = re.sub(r'[ \t]+', ' ', code)
    return code

def extract_declarations(code):
    """
    Walk through the (normalized) code character by character.
    When a top-level '{' is encountered, backtrack (from buf) to capture the header.
    If the header contains a ")" (and doesn’t start with struct/union/enum),
    assume it’s a function and output the header + ";".
    Otherwise, output the full block.
    Top-level statements ending with ';' are also captured.
    """
    decls = []
    buf = ""
    level = 0
    i = 0
    n = len(code)
    while i < n:
        c = code[i]
        if c == '{':
            if level == 0:
                header = buf.strip()
                buf = ""
                # Heuristic: if header contains a ")" and doesn’t start with struct/union/enum,
                # assume it’s a function definition.
                is_function = (')' in header and
                               not header.startswith("struct") and
                               not header.startswith("union") and
                               not header.startswith("enum"))
                block = "{"
                level = 1
                i += 1
                while i < n and level > 0:
                    ch = code[i]
                    block += ch
                    if ch == '{':
                        level += 1
                    elif ch == '}':
                        level -= 1
                    i += 1
                full_decl = header + " " + block
                if is_function:
                    decls.append(header + ";")
                else:
                    # (Optionally skip enums; adjust if needed)
                    if header.startswith("enum"):
                        pass
                    else:
                        decls.append(full_decl)
                continue  # already advanced i
            else:
                buf += c
                level += 1
        elif c == '}':
            buf += c
            level -= 1
        elif c == ';' and level == 0:
            buf += c
            decls.append(buf.strip())
            buf = ""
        else:
            buf += c
        i += 1
    return decls

def main():
    if len(sys.argv) != 2:
        sys.exit("Usage: extract_prototypes.py <source_file>")
    with open(sys.argv[1], 'r') as f:
        code = f.read()
    code = remove_comments(code)
    code = normalize_code(code)
    decls = extract_declarations(code)
    for d in decls:
        print(d)

if __name__ == '__main__':
    main()
