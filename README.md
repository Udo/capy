# Capy, a C-like

## C compatibility

Relatively normal C code should compile without changes.

## -> Operator optional

```C
void do_something(MyStruct* m)
{
	printf("Name: %s\n", m.name);
}
```

You can use the `.` operator instead of `->` when accessing members of struct pointers.

## structs

- The `struct` keyword can be omitted when declaring a struct variable.

## Compiler directives

- `#product`: specifies the executable filename (e.g. `#product "compiled_file_name"`)
- `#link`: link a library file, equivalent to `-l` command line switch (e.g. `#link "SDL2"`)
- `#library`: library path, equivalent to `-L` command line switch (e.g. `#library "/usr/lib/x86_64-linux-gnu/pulseaudio/"`)

## enums

- runtime information: enums tagged with `[@ext]` generate arrays called `MYENUM:names[]` and `MYENUM:values[]` (where MYENUM is the name of the enum), and these contain the names and values of all enum constants as a string available at runtime.
- *iota counter*: you can use the Go-like counter `iota` in enum and struct definitions (like `__COUNTER__` but more useful). The counter is reset to 0 when a new scope or enum definition begins.

## Minor things

- *Multiline strings*: string literals can contain (unescaped) line breaks
- *Default executable name*: default binary output name same as first filename argument minus extension
- *Sane basic type names*: rust-style basic types u8, u16, s16, f16, u32, s32, f32, u64, s64, f64, u128, s128, s128
- *attribute syntax*: `[@attrib]`, for example `[@section("name")]`

## Runtime type information

- *`typeid()`* for runtime access to a numeric type ID

# Install

## Compile

Execute `./configure` before compiling for the first time. Afterwards, build the executable by using the `./build` script.
