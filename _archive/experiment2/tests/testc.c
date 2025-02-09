#include <stdio.h>
#include <stdlib.h>

struct MyStruct {
	int a;
	int b;
	int c;
};

int main(int argc, char** argv) {

	MyStruct(123); // this crashes TCC
	printf("hello world\n");

}

