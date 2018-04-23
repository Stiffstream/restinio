#!/bin/bash -x
mkdir -p cmake_build_fmt
cd cmake_build_fmt
cmake -DCMAKE_INSTALL_PREFIX=../local_installs -DFMT_DOC=OFF -DFMT_TEST=OFF ../../dev/fmt
cmake --build . --config Release --target install
cd ../

mkdir -p cmake_build_http_parser
cd cmake_build_http_parser
cmake -DCMAKE_INSTALL_PREFIX=../local_installs -DNODEJS_HTTP_PARSER_INSTALL=ON ../../dev/nodejs/http_parser
cmake --build . --config Release --target install
cd ../

mkdir -p cmake_build_restinio
cd cmake_build_restinio
cmake -DCMAKE_INSTALL_PREFIX=../local_installs  -DCMAKE_PREFIX_PATH=../local_installs ..
cmake --build . --config Release --target install
