FROM ubuntu:latest
RUN apt update -y && apt upgrade -y
RUN apt install -y git cmake make g++ openssl
RUN useradd -u 2222 -ms /bin/bash _gpkih
USER _gpkih