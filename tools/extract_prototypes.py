#!/usr/bin/env python3
import sys, re

# this script is an AI-generated nightmare that is pretty much unmaintainable even by the AI that created it

def remove_comments(code):
    # Remove C-style (/* */) and C++-style (//) comments.
    code = re.sub(r'/\*.*?\*/', '', code, flags=re.DOTALL)
    code = re.sub(r'//.*', '', code)
    return code

def normalize_code(code):
    # Collapse spaces and tabs (but keep newlines intact).
    code = re.sub(r'[ \t]+', ' ', code)
    return code

def extract_declarations(code):
    """
    Extract top-level declarations:
      - Function definitions are converted to prototypes (header + ";")
      - Other top-level statements ending with ';' or blocks are output as-is.
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
                # Heuristic: if header contains ')' and doesn't start with struct/union/enum,
                # assume it's a function definition.
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
                    # Optionally skip enums; adjust if needed.
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
            if buf.strip() != ";":
                decls.append(buf.strip())
            buf = ""
        else:
            buf += c
        i += 1
    return decls

def transform_decl(decl):
    decl_stripped = decl.strip()
    if decl_stripped == "" or decl_stripped == ";":
        return ""
    # Leave preprocessor directives unchanged.
    if decl_stripped.startswith("#"):
        return decl
    # Skip static declarations.
    if decl_stripped.startswith("static"):
        return ""
    # Already declared extern or typedef.
    if decl_stripped.startswith("extern") or decl_stripped.startswith("typedef"):
        return decl
    # Heuristic for function prototypes:
    # if it contains '(' (and not "(*") and does not include an initializer,
    # assume it's a function declaration.
    if "=" not in decl_stripped and "(" in decl_stripped and "(*" not in decl_stripped:
        return decl

    # NEW: Check for an initializer first. If found, strip it.
    if "=" in decl_stripped:
        decl_no_init = decl_stripped.split('=', 1)[0].strip()
        result = "extern " + decl_no_init
        if not result.endswith(";"):
            result += ";"
        return result

    # Now handle block definitions that don't have an '='.
    if "{" in decl_stripped and "}" in decl_stripped:
        # For enums, structs, and unions, leave them unchanged (just ensure trailing semicolon).
        if (decl_stripped.startswith("enum") or
            decl_stripped.startswith("struct") or
            decl_stripped.startswith("union")):
            result = decl_stripped.rstrip()
            if not result.endswith(";"):
                result += ";"
            return result
        else:
            # Other block definitions: just ensure trailing semicolon.
            result = decl_stripped.rstrip()
            if not result.endswith(";"):
                result += ";"
            return result

    # Fallback.
    result = "extern " + decl_stripped
    if not result.endswith(";"):
        result += ";"
    return result

def main():
    if len(sys.argv) != 2:
        sys.exit("Usage: extract_prototypes.py <source_file>")
    with open(sys.argv[1], 'r') as f:
        code = f.read()
    code = remove_comments(code)
    code = normalize_code(code)
    decls = extract_declarations(code)
    transformed_decls = [transform_decl(d) for d in decls]
    transformed_decls = [d for d in transformed_decls if d.strip()]
    # Join all declarations into one output string.
    output = "\n".join(transformed_decls)
    # Fix: if a declaration ends with '}' and the next starts with 'extern',
    # remove the extra 'extern ' so that a typedef enum is properly combined.
    output = re.sub(r'}\s*\nextern\s+', '} ', output)
    print(output)


if __name__ == '__main__':
    main()
