#!/bin/bash

clang-format -i *.c -style "{UseTab: Always, IndentWidth: 4, TabWidth: 4}"
clang-format -i *.h -style "{UseTab: Always, IndentWidth: 4, TabWidth: 4}"
