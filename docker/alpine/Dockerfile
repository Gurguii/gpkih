FROM alpine:latest
RUN apk update && apk upgrade
RUN apk add git cmake make g++ bash openssl
RUN adduser -s /bin/bash -D _gpkih
USER _gpkih