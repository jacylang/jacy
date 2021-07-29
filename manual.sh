cmake -B cmake-build-debug -D CMAKE_CXX_COMPILER=clang++ && make -C ./cmake-build-debug && ./cmake-build-debug/bin/Jacy "$@"
