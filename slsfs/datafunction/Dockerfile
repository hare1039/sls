FROM alpine:3.16 AS builder

RUN apk update && \
    apk add --no-cache \
        py3-pip make cmake ccache git wget bash g++ gcc linux-headers perl \
        zlib-dev openssl-dev zstd-dev zstd-static && \
    pip install conan

RUN git clone --branch v2021.7.0 https://github.com/oneapi-src/oneTBB.git && \
    mkdir -p /oneTBB/build && cd /oneTBB/build && \
    cmake -DTBB_TEST=OFF -DBUILD_SHARED_LIBS=OFF .. && \
    make && make install

RUN conan profile new default --detect &&\
    conan profile update settings.compiler.libcxx=libstdc++11 default

ADD datafunction/profiles      /pre/profiles
ADD datafunction/conanfile.txt /pre

RUN mkdir /pre/build && cd /pre/build && \
    conan install .. --profile ../profiles/release --build=cascade --build=librdkafka --build=cassandra-cpp-driver --build=poco --build=openssl --build=m4 && \
    conan install .. --profile ../profiles/debug   --build=cascade --build=librdkafka --build=cassandra-cpp-driver --build=poco --build=openssl --build=m4

ADD libslsfs /action/libslsfs

ADD datafunction /action/

ARG debug

RUN --mount=type=cache,target=/action/build mkdir -p /action/build && \
    cd /action/build && \
    bash -c 'if [[ -z "$debug" ]]; then conan install .. --profile ../profiles/release --build missing; else conan install .. --profile ../profiles/debug --build missing; fi' && \
    bash -c 'if [[ -z "$debug" ]]; then cmake .. -DCMAKE_BUILD_TYPE=Release; else cmake .. -DCMAKE_BUILD_TYPE=Debug; fi' && \
    cmake --build . && \
    cp /action/build/bin/exec /action/

#RUN --mount=type=cache,target=/ccache cd /action && time ccache g++ -std=c++20 -o exec -static *.cpp \
#    -DFMT_HEADER_ONLY \
#    $CPPBUILDARGS \
#    -I/action/libslsfs/ -I/action/libslsfs/slsfs \
#    -L/usr/local/lib/ -pthread \
#    -lcassandra_static \
#    -lrdkafka++ -lrdkafka -luv \
#    -lcurl -lssl -lnghttp2 -lz -lzstd -lbrotlienc -lbrotlidec -lbrotlicommon \
#    -lPocoFoundation \
#    -lboost_coroutine -lboost_chrono -lboost_iostreams -lboost_thread -lboost_system

FROM hare1039/dockerskeleton:latest

COPY --from=builder /action/exec /action/exec

ENV FLASK_PROXY_PORT 8080

CMD ["/bin/bash", "-c", "cd actionProxy && python -u actionproxy.py"]
