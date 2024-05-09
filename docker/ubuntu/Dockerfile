FROM ubuntu:latest
RUN apt update -y && apt upgrade -y
RUN apt install -y git cmake make g++ openssl
RUN useradd -ms /bin/bash _gpkih
USER _gpkih