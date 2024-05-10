FROM archlinux:latest
RUN pacman -Syu --noconfirm
RUN pacman -S --noconfirm git cmake make gcc openssl
RUN useradd -u 2222 -ms /bin/bash _gpkih
USER _gpkih