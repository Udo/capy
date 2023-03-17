#include <stdio.h>
#include <stdlib.h>
#include "capy1.types.playground.c"
#product "bin/test_capy1"

struct MyTest {
	u32 a;
	f32 f;
};

int main(int argc, char** argv) {

	MyTest* m =
		malloc(sizeof(MyTest));
	m.a = 123;

	MyTest m2;
	m2->f = 10; //string_create_with_chars("321");
	u8* mf = <><html>
		let's goooo! <?= m2.f ?>
		</html></>;

	printf("hello world %i, %i, typeid=%i, typeid=%i, typeid=%i, typeid=%i %s\n", m.a, m2.a,
		typeid(MyTest), typeid(int), typeid(*m), sizeof(u32), __FILE__);

	printf("here are some floats %.3f\n",
		1.23450);

	printf("here are some strings %s\n",
		mf);

    return 0;
}

/* DONE

*	. and -> can now be used interchangedly
*	don't need to use the "struct" keyword in variable decl
*	added typeid() for runtime access to a type ID
*	multiline strings (was already a thing in TCC!)
*	default binary output name same as first filename argument minus extension
*	#product "compiled_file_name"
*	assignment warnings now actually show the types that are in conflict in verify_assign_cast()
*	assigning a struct pointer to a basic type without casting is now an error
*	default to sane basic type names (u8 etc) [sort of, just typedefs for now]

OPEN
*	change compiler flags with # directives
*	capy strings
*	string interpolation
*	function signature matching
*	type reflection at runtime
*	polymorphism
*	compile-time code execution / metaprogramming
*	defer things for when symbols go out of scope?
*	generics?
*	var keyword for type inference?

*/
