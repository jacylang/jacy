# Ownership problems

```c
struct A {};

int * a = new A();
int * b = a;

free(b); // memory that `b` pointed too is now freed as the memory of `a`
```

```cpp
struct A {
    int value = 100;
};

struct B : A {};

auto a = new B();
auto b = a;
```
