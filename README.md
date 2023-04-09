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

## Function Signature Matching

```C
// Capy function declaration starts with "fn"
fn bool equals(u8* str1, u8* str2)
{
	return strcmp(str1, str2) == 0;
}

fn bool equals(u8* str1, s32 i2)
{
	return atoi(str1) == i2;
}

printf("Let's compare: %s == %i ? %i", "123", 123, equals("123", 123));
```

To preserve backwards compatibility to C, Capy introduces an alternative way to declare functions, starting with the `fn` keyword. Capy functions will pattern-match according to the parameter types at the call site.

## structs

- The `struct` keyword can be omitted when declaring a struct variable.
- runtime information: structs tagged with `[@ext]` generate arrays called `MYSTRUCT:name[]` (where MYSTRUCT is the name of the struct), and these contain the names of all members as a string available at runtime.

## Compiler directives

- `#product`: specifies the executable filename (e.g. `#product "compiled_file_name"`). In general, you should not need a build system when writing software with Capy. Being based on TCC, the compiler is fast enough for most projects to compile in a single pass within miliseconds.
- `#link`: link a library file, equivalent to `-l` command line switch (e.g. `#link "SDL2"`)
- `#library`: library path, equivalent to `-L` command line switch (e.g. `#library "/usr/lib/x86_64-linux-gnu/pulseaudio/"`)

## enums

- runtime information: enums generate arrays called `MYENUM:name[]` and `MYENUM:value[]` (where MYENUM is the name of the enum), and these contain the names and values of all enum constants as a string available at runtime.
- *iota counter*: you can use the Go-like counter `iota` in enum and struct definitions (like `__COUNTER__` but more useful). The counter is reset to 0 when a new scope or enum definition begins.

## Minor things

- *Multiline strings*: string literals can contain (unescaped) line breaks
- *Default executable name*: default binary output name same as first filename argument minus extension
- *Sane basic type names*: rust-style basic types u8, u16, s16, f16, u32, s32, f32, u64, s64, f64, u128, s128, s128
- *attribute syntax*: `[@attrib]`, for example `[@section("name")]`
- `countof(ARRAY)` returns the number of elements in the array

## Runtime type information

- *`typeid()`* for runtime access to a numeric type ID

# Install

## Compile

Prerequisites: you need to install TCC on the system before using capy, since Capy is using system-wide TCC libraries.

Execute `./configure` before compiling for the first time. Afterwards, build the executable by using the `./build` script.
