# Static Chaos MUD -- containerized build.
#
# Base image: Debian Bullseye (gcc 10). Deliberately not the newest gcc:
# gcc 14 turns this 1990s code's implicit function declarations and
# implicit-int into hard errors. gcc 10 keeps them as warnings, so the
# legacy Merc/Diku source compiles with only the Makefile tweaks
# (-fcommon, -lcrypt) we made.
FROM debian:bullseye-slim

# build-essential provides gcc + make. (No libcrypt: passwords are plaintext
# on Linux in this codebase -- see merc.h -- so crypt() is never linked.)
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
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
