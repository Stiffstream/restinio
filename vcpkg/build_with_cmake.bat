mkdir cmake_build_fmt
cd cmake_build_fmt
cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=../local_installs -DFMT_DOC=OFF -DFMT_TEST=OFF ../../dev/fmt
nmake install
cd ../

mkdir cmake_build_http_parser
cd cmake_build_http_parser
cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=../local_installs -DNODEJS_HTTP_PARSER_INSTALL=ON ../../dev/nodejs/http_parser
nmake install
cd ../

mkdir cmake_build_restinio
cd cmake_build_restinio
cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=../local_installs  -DCMAKE_PREFIX_PATH=../local_installs ..
nmake install
PAUSE
