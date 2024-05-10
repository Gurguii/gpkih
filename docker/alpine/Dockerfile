FROM alpine:latest
RUN apk update && apk upgrade
RUN apk add git cmake make g++ bash openssl shadow
RUN useradd -u 2222 -ms /bin/bash _gpkih
USER _gpkih