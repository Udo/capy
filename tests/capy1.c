#include <stdio.h>
#include <stdlib.h>
#include "../src/capy.h"
#include "../use/dtree.h"
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

enum week {Mon = iota, Tue = iota, Wed = iota, Thur, Fri, Sat, Sun};

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

fn u8 wonk()
{
	return 0;
}

fn u32 wonk()
{
	return 0;
}

int main(int argc, char** argv) {

	"[@Prefix]";

	MyTest ms = { .a = 123 };

	MyTest* m =
		malloc(sizeof(MyTest));
	using ms;
	a = 123;
	f = 10.1;

	*m = ms;

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

	u64 h = 10101;
	for(u32 i = 0; i < 1024*1024; i++)
		h = u64_hash(h);

	week day = Wed;
    printf("Enum test: %d = %s = %i\n", day, week:name[day], week:value[day]);

    dtree* d1 = dtree:create();

    return 0;
}


