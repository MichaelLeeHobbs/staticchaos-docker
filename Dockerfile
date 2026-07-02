# Static Chaos MUD -- containerized build.
#
# Base image: Debian Bullseye (gcc 10). Deliberately not the newest gcc:
# gcc 14 turns this 1990s code's implicit function declarations and
# implicit-int into hard errors. gcc 10 keeps them as warnings, so the
# legacy Merc/Diku source compiles with only the Makefile tweaks
# (-fcommon, -lcrypt) we made.
# Pinned by digest for reproducibility. Tag: debian:bullseye-slim.
# Re-pin: docker pull debian:bullseye-slim && \
#   docker inspect --format='{{index .RepoDigests 0}}' debian:bullseye-slim
FROM debian:bullseye-slim@sha256:f18adf4e1d04b1d8ba48025b8e35003f4c748ddd3dd8e875fe4e7d9a9c0dec84

# build-essential provides gcc + make. gcc-multilib + libc6-dev-i386 provide the
# 32-bit toolchain and libs: the server is built -m32 because this code assumes
# 32-bit pointers and crashes during gameplay when built 64-bit (see src/Makefile).
# (No libcrypt: passwords are plaintext on Linux in this codebase -- see merc.h.)
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        gcc-multilib \
        libc6-dev-i386 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /mud
COPY . /mud

# Compile the `chaosium` server binary.
RUN make -C src clean || true \
    && make -C src

# The server saves player files to ../player/<initial>/<Name> and uses a
# ../player/temp scratch dir (see src/save.c). fopen("w") will not create
# missing directories, so pre-create a-z plus temp. Also ensure a log dir.
RUN set -eux; \
    for d in a b c d e f g h i j k l m n o p q r s t u v w x y z; do \
        mkdir -p "/mud/player/$d"; \
    done; \
    mkdir -p /mud/player/temp /mud/log

# The MUD's telnet listener. Code requires the port to be > 1024.
EXPOSE 4000

# The original launcher runs the binary from the area/ directory, because the
# server loads area.lst and other game data via paths relative to cwd.
WORKDIR /mud/area
CMD ["/mud/src/chaosium", "4000"]
