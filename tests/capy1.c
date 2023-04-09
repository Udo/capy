#include <stdio.h>
#include <stdlib.h>
#include "../src/capy.h"
#product "bin/test_capy1"

#define fn	[@capyfn]

struct [@ext] MyTest {
	u32 a;
	f32 f;
	struct SubStruct {
		u32 sa;
		f32 sf;
	} s;
};
/*u8* MyTest:name[] = { "a", "f", };
s32 MyTest:typeid[] = { 123, 234, };
s32 MyTest:offset[] = { 0, 4, };*/

enum [@ext] week {Mon = iota, Tue = iota, Wed = iota, Thur, Fri, Sat, Sun};
//u8* week:names[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

fn b8 equals(char* a, char* b)
{
	printf("EQUALS EXEC: CHAR* (%s, %s)\n", a, b);
	return(true);
}

fn b8 equals(u32 a, s32 b)
{
	printf("EQUALS EXEC: U32 (%i, %i)\n", a, b);
	return(a-b);
}

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
	string* mystr = string_create_with_chars("hello world");

	for(s32 i = 0; i < countof(MyTest:name); i++)
		printf("MyTest field %i/%i = %s\n", i, countof(MyTest:name), MyTest:name[i]);

	printf("hello world %i, %i, typeid=%i, typeid=%i, typeid=%i, typeid=%i %s\n", m.a, m2.a,
		typeid(MyTest), typeid(int), typeid(*m), sizeof(u32), MyTest:name[0]);

	printf("eq test %i\n",
		equals("aaaa", "bb"));

	printf("ext: %p\n",
		(s32)hash_into_u64("ext", 3));
	printf("capyfn: %p\n",
		(s32)hash_into_u64("capyfn", 6));

	week day = Wed;
    printf("Enum test: %d = %s = %i\n", day, week:name[day], week:value[day]);

    return 0;
}


