#!/usr/bin/env python3
import re
import sys

def remove_comments(code):
    code = re.sub(r'//.*', '', code)
    code = re.sub(r'/\*.*?\*/', '', code, flags=re.DOTALL)
    return code

def extract_typedef_blocks(code):
    """Extract typedef blocks (struct/enum/union) and replace with placeholders."""
    typedef_blocks = {}
    pattern = re.compile(r'\btypedef\s+(struct|enum|union)\b')
    i = 0
    out = []
    placeholder_index = 0
    while i < len(code):
        match = pattern.search(code, i)
        if not match:
            out.append(code[i:])
            break
        start = match.start()
        out.append(code[i:start])
        brace_index = code.find('{', match.end())
        if brace_index == -1:
            out.append(code[match.start():])
            break
        j = brace_index
        brace_level = 0
        while j < len(code):
            if code[j] == '{':
                brace_level += 1
            elif code[j] == '}':
                brace_level -= 1
                if brace_level == 0:
                    break
            j += 1
        if j >= len(code):
            break
        semicolon_index = code.find(';', j)
        if semicolon_index == -1:
            break
        block = code[start:semicolon_index+1]
        placeholder = f"__TYPEDEF_PLACEHOLDER_{placeholder_index}__"
        typedef_blocks[placeholder] = block
        placeholder_index += 1
        out.append(placeholder)
        i = semicolon_index + 1
    return "".join(out), typedef_blocks

def reinsert_typedef_blocks(code, typedef_blocks):
    for placeholder, block in typedef_blocks.items():
        code = code.replace(placeholder, block)
    return code

def skip_string(code, i, quote_char):
    i += 1
    n = len(code)
    while i < n:
        if code[i] == '\\':
            i += 2
        elif code[i] == quote_char:
            i += 1
            break
        else:
            i += 1
    return i

def remove_function_bodies(code):
    """
    Convert function definitions to prototypes by removing their bodies.
    When encountering a '{', if the immediately preceding non-space character is ')'
    and the signature snippet (from the previous ';' or newline) doesn't contain
    a type-def keyword, treat it as a function definition.
    """
    out = []
    i = 0
    n = len(code)
    while i < n:
        c = code[i]
        if c in ('"', "'"):
            start = i
            i = skip_string(code, i, c)
            out.append(code[start:i])
            continue
        if c == '{':
            j = i - 1
            while j >= 0 and code[j].isspace():
                j -= 1
            if j >= 0 and code[j] == ')':
                k = j
                while k >= 0 and code[k] not in ";\n":
                    k -= 1
                signature = code[k+1:j+1]
                if re.search(r'\b(struct|enum|union)\b', signature):
                    out.append("{")
                    i += 1
                    continue
                out.append(";")
                i += 1
                brace_level = 1
                while i < n and brace_level:
                    c2 = code[i]
                    if c2 in ('"', "'"):
                        i = skip_string(code, i, c2)
                        continue
                    elif c2 == '{':
                        brace_level += 1
                    elif c2 == '}':
                        brace_level -= 1
                    i += 1
                continue
            else:
                out.append("{")
                i += 1
                continue
        else:
            out.append(c)
            i += 1
    return "".join(out)

def process_variable_declarations(code):
    """
    Transform global variable definitions with initializers into extern declarations.
    This handles both simple initializers and compound initializers spanning multiple lines.
    Example:
      operator_table_def operator_table[] = { ... };
    becomes:
      extern operator_table_def operator_table[];
    """
    # Match declarations with an initializer (which can span multiple lines)
    pattern = re.compile(
        r'^(?!\s*extern\b)([^\n;]*?\b)(\w+\s*(?:\[[^]]*\])?)\s*=\s*.*?;',
        re.MULTILINE | re.DOTALL)
    def repl(m):
        decl = m.group(1)
        var = m.group(2)
        return f"extern {decl}{var};"
    return pattern.sub(repl, code)

def deduplicate_declarations(code):
    """Remove duplicate function prototypes generated from forward declarations."""
    lines = code.splitlines()
    normalized = []
    i = 0
    while i < len(lines):
        if lines[i].rstrip().endswith(")") and (i+1 < len(lines)) and lines[i+1].strip() == ";":
            normalized.append(lines[i].rstrip() + ";")
            i += 2
        else:
            normalized.append(lines[i])
            i += 1
    seen = set()
    output = []
    prototype_pattern = re.compile(r'^\s*(?:extern\s+)?[^(]+\([^)]*\)\s*;')
    for line in normalized:
        if prototype_pattern.match(line):
            key = line.strip()
            if key in seen:
                continue
            seen.add(key)
        output.append(line)
    return "\n".join(output)

def main(filename):
    with open(filename, 'r') as f:
        code = f.read()
    code = remove_comments(code)
    code, typedef_blocks = extract_typedef_blocks(code)
    code = remove_function_bodies(code)
    code = process_variable_declarations(code)
    code = deduplicate_declarations(code)
    code = reinsert_typedef_blocks(code, typedef_blocks)
    header = (
        code
    )
    print(header)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        sys.exit("Usage: python script.py <c_file>")
    main(sys.argv[1])
