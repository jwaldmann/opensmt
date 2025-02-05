#!/bin/bash

set -ev
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_CXX_FLAGS="$FLAGS" -DUSE_READLINE:BOOL=${USE_READLINE} -DCMAKE_INSTALL_PREFIX=${INSTALL} -DPACKAGE_BENCHMARKS=${PACKAGE_BENCHMARKS} ..
make -j4
make test
make install
if [[ ${CMAKE_BUILD_TYPE} == Debug ]]; then
    cd ../regression && ./run-test-notiming.sh ../build/opensmt;
    cd ../regression_itp && ./run-tests.sh ../build/opensmt;
    cd ../regression_splitting && ./bin/run-tests.sh ../build/opensmt;
fi

cd ../examples && rm -rf build && mkdir -p build && cd build
if [[ ${CMAKE_BUILD_TYPE} == Debug ]]; then
    cmake \
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
        -DOpenSMT_DIR=${INSTALL} \
        -DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined \
        ..
else
    cmake \
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
        -DOpenSMT_DIR=${INSTALL} \
        ..
fi

make -j4

