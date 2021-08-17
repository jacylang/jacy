#include <iostream>

struct S {
    std::string field;
};

static inline const S & get() {
    static S s{"pwierewriewrobweorienwoi"};
    return s;
}

int main() {
    std::cout << get().field;

    return 0;
}
