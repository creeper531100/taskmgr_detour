#include <cstdio>

#include "pch.h"

template <>
float foo(float a) {
    return a + 0.1;
}

template int foo(int);
template float foo(float);

int foo_func() {
    printf("HelloWorld");
    return 0;
}