FROM ubuntu:18.04

# Prepare build environment
RUN apt-get update && \
    apt-get -qq -y install gcc g++ ruby \
    wget libpcre2-dev libpcre3-dev pkg-config \
	 libboost-all-dev \
	 libssl-dev \
    libtool
RUN gem install Mxx_ru

RUN mkdir /tmp/restinio
COPY externals.rb /tmp/restinio
COPY dev /tmp/restinio/dev

RUN echo "*** Extracting RESTinio's Dependencies ***" \
	&& cd /tmp/restinio \
	&& mxxruexternals

RUN echo "*** Building RESTinio ***" \
	&& cd /tmp/restinio/dev \
	&& MXX_RU_CPP_TOOLSET=gcc_linux ruby build.rb --mxx-cpp-release

