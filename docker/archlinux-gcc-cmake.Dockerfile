FROM archlinux/base:latest

# Prepare build environment
RUN pacman -Sy --noconfirm gcc \
	&& pacman -Sy --noconfirm ruby rubygems \
	&& pacman -Sy --noconfirm wget 

RUN gem install Mxx_ru

RUN mkdir /tmp/restinio
COPY externals.rb /tmp/restinio
COPY dev /tmp/restinio/dev

RUN echo "*** Extracting RESTinio's Dependencies ***" \
	&& export PATH=${PATH}:~/.gem/ruby/2.6.0/bin \
	&& cd /tmp/restinio \
	&& mxxruexternals

RUN echo "*** Getting CMake ***" \
	&& pacman -Sy --noconfirm cmake make

RUN echo "*** Building RESTinio ***" \
	&& cd /tmp/restinio/dev \
	&& mkdir cmake_build \
	&& cd cmake_build \
	&& cmake -DCMAKE_INSTALL_PREFIX=target -DCMAKE_BUILD_TYPE=Release .. \
	&& cmake --build . --config Release \
	&& cmake --build . --target test

