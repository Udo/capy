#include <stdio.h>
#include <stdlib.h>
#include "capy1.types.playground.c"
#product "bin/test_capy1"

struct MyTest {
	u32 a;
	f32 f;
};

enum [@ext] week {Mon = iota, Tue = iota, Wed = iota, Thur, Fri, Sat, Sun};
//u8* week:names[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

int main(int argc, char** argv) {

	"[@Prefix]";

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

	printf("scope ID: %i\n",
		scopeid(0));

	printf("ext: %p\n",
		(s32)hash_into_u64("ext", 3));

	week day = Wed;
    printf("Enum test: %d = %s = %i\n", day, week:names[day], week:values[day]);

    return 0;
}

