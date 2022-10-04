FROM hare1039/dockerskeleton:latest AS builder

RUN apk update && \
    apk add --no-cache --virtual .build-deps \
                                 bzip2-dev \
                                 g++ \
                                 libc-dev \
                                 cassandra-cpp-driver-dev \
                                 openssl-libs-static zlib-static cyrus-sasl-static \
                                 curl-dev curl-static nghttp2-static zstd-static libuv-static brotli-static \
                                 boost-dev boost-static ccache \
                                 make

RUN git clone --branch CRYPTOPP_8_7_0 https://github.com/weidai11/cryptopp.git /cryptopp && \
    cd /cryptopp && CXXFLAGS="-DNDEBUG -g2 -O3 -std=c++11" make -j && make install

RUN git clone --branch v1.8.2 https://github.com/edenhill/librdkafka.git /librdkafka && \
    cd /librdkafka && ./configure --enable-static --disable-sasl --install-deps && \
    make && make install

ADD libslsfs /action/libslsfs

ADD datafunction /action/

#RUN cd /action && time g++ -O3 -std=c++17 -c -o slsfs.o -static *.cpp \
#    -I/action/libslsfs/ -I/action/libslsfs/slsfs -DFMT_HEADER_ONLY -DAS_LIBSLSFS
#
#RUN cd /action; bash ./build-libslsfs.sh

ARG CPPBUILDARGS
ENV CCACHE_DIR=/ccache

RUN --mount=type=cache,target=/ccache cd /action && time ccache g++ -std=c++20 -o exec -static *.cpp \
    -DFMT_HEADER_ONLY \
    $CPPBUILDARGS \
    -I/action/libslsfs/ -I/action/libslsfs/slsfs \
    -L/usr/local/lib/ -pthread \
    -lcryptopp -lcassandra_static \
    -lrdkafka++ -lrdkafka -luv \
    -lcurl -lssl -lcrypto -lnghttp2 -lz -lzstd -lbrotlienc -lbrotlidec -lbrotlicommon \
    -lboost_coroutine -lboost_chrono -lboost_iostreams -lboost_thread -lboost_system

FROM hare1039/dockerskeleton:latest

COPY --from=builder /action/exec /action/exec
#COPY --from=builder /action/libs /action/libs/

ENV FLASK_PROXY_PORT 8080

CMD ["/bin/bash", "-c", "cd actionProxy && python -u actionproxy.py"]
