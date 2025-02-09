#pragma once

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct dtree {

};

dtree* dtree:create() {
	return(malloc(sizeof(dtree)));
}

