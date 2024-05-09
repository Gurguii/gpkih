FROM archlinux:latest
RUN pacman -Syu --noconfirm
RUN pacman -S --noconfirm git cmake make gcc openssl
RUN useradd -ms /bin/bash _gpkih
USER _gpkih