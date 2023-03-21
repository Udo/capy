#include <stdio.h>
#include <stdlib.h>
#include "capy1.types.playground.c"
#product "bin/test_capy1"

struct MyTest {
	u32 a;
	f32 f;
};

enum week {Mon = iota, Tue = iota, Wed = iota, Thur, Fri, Sat, Sun};
enum week2 {Mon2 = iota, Tue2 = iota, Wed2 = iota, Thur2, Fri2, Sat2, Sun2};
u8* week:names[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

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

	printf("here are some strings %s\n",
		mf);

	week day = Wed2;
    printf("%d = %s %i %i\n", day, week:names[day], __COUNTER__, __COUNTER__);

    return 0;
}

