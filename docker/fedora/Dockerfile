FROM fedora:latest
RUN dnf update -y && dnf upgrade -y
RUN dnf install -y git cmake make g++ openssl
RUN useradd -ms /bin/bash _gpkih
USER _gpkih