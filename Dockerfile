FROM grpc/cxx:latest
MAINTAINER Deshna Jain <deshnazen@gmail.com>

RUN apt-get update
RUN apt-get -y install clang libgtest-dev cmake

RUN cd /usr/src/gtest && cmake . && make -j5 && cp /usr/src/gtest/*.a /usr/local/lib

COPY prefix_service $HOME/prefix_service
WORKDIR $HOME/prefix_service

RUN make -j5
RUN ./prefix_server
EXPOSE 8080

