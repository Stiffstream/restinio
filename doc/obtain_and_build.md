# Obtain And Build

## Prerequisites

To use *RESTinio* it is necessary to have:

* Reasonably modern C++14 compiler (VC++14.0, GCC 5.4 or above, clang 3.8 or above);
* [asio](http://think-async.com/Asio) from [git repo](https://github.com/chriskohlhoff/asio.git), commit `f5c570826d2ebf50eb38c44039181946a473148b`;
* [nodejs/http-parser](https://github.com/nodejs/http-parser) 2.7.1;
* [fmtlib](http://fmtlib.net/latest/index.html) 4.0.0.
* Optional: [SObjectizer](https://sourceforge.net/projects/sobjectizer/) 5.5.19.3;

For building samples, benchmarks and tests:

* [Mxx_ru](https://sourceforge.net/projects/mxxru/) 1.6.13 or above;
* [rapidjson](https://github.com/miloyip/rapidjson) 1.1.0;
* [json_dto](https://bitbucket.org/sobjectizerteam/json_dto-0.1) 0.1.2.1 or above;
* [args](https://github.com/Taywee/args) 6.0.4;
* [CATCH](https://github.com/philsquared/Catch) 1.9.6.

## Obtaining

There are two ways of obtaining *RESTinio*.

* Getting from
[repository](https://bitbucket.org/sobjectizerteam/restinio-0.3).
In this case external dependencies must be obtained with Mxx_ru externals tool.
* Getting
[archive](https://bitbucket.org/sobjectizerteam/restinio-0.2/downloads/restinio-0.2.1-full.tar.bz2).
Archive includes source code for all external dependencies.

### Cloning of hg repository

```
hg clone https://bitbucket.org/sobjectizerteam/restinio-0.3
```

And then:
```
cd restinio-0.3
mxxruexternals
```
to download and extract *RESTinio*'s dependencies.

### MxxRu::externals recipe

See MxxRu::externals recipes for *RESTinio*
[here](./doc/MxxRu_externals_recipe.md).

### Getting archive

```
wget https://bitbucket.org/sobjectizerteam/restinio-0.3/downloads/restinio-0.3.0-full.tar.bz2
tar xjvf restinio-0.3.0-full.tar.bz2
cd restinio-0.3.0-full
```

## Build

### CMake

Building with CMake currently is provided for samples, tests and benches
not depending on SObjectizer.
To build them run the following commands:
```
hg clone https://bitbucket.org/sobjectizerteam/restinio-0.3
cd restinio-0.3
mxxruexternals
cd dev
mkdir cmake_build
cd cmake_build
cmake -DCMAKE_INSTALL_PREFIX=target -DCMAKE_BUILD_TYPE=Release ..
make
make install
```

Or, if getting sources from archive:
```
wget https://bitbucket.org/sobjectizerteam/restinio-0.3/downloads/restinio-0.3.0-full.tar.bz2
tar xjvf restinio-0.3.0-full.tar.bz2
cd restinio-0.3.0-full/dev
mkdir cmake_build
cd cmake_build
cmake -DCMAKE_INSTALL_PREFIX=target -DCMAKE_BUILD_TYPE=Release ..
make
make install
```

### Mxx_ru
While *RESTinio* is header-only library, samples, tests and benches require a build.

Compiling with Mxx_ru:
```
hg clone https://bitbucket.org/sobjectizerteam/restinio-0.3
cd restinio-0.3
mxxruexternals
cd dev
ruby build.rb
```

For release or debug builds use the following commands:
```
ruby build.rb --mxx-cpp-release
ruby build.rb --mxx-cpp-debug
```

*NOTE.* It might be necessary to set up `MXX_RU_CPP_TOOLSET` environment variable,
see Mxx_ru documentation for further details.

### Dependencies default settings

External libraries used by *RESTinio* have the following default settings:

* A standalone version of *asio* is used and a chrono library is used,
so `ASIO_STANDALONE` and `ASIO_HAS_STD_CHRONO` defines are necessary. Also
`ASIO_DISABLE_STD_STRING_VIEW` is defined, btcause it is a C++17 feature and
not all compilers support it yet;
* a less strict version of *nodejs/http-parser* is used, so the following
definition `HTTP_PARSER_STRICT=0` is applied;
* *fmtlib* is used as a header-only library, hence a `FMT_HEADER_ONLY`
define is necessary;
* for *RapidJSON* two definitions are necessary: `RAPIDJSON_HAS_STDSTRING` and
`RAPIDJSON_HAS_CXX11_RVALUE_REFS`.
