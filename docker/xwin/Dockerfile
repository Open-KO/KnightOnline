FROM ubuntu:25.10 AS builder

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        mingw-w64 \
        git \
        ca-certificates \
        cmake \
        wget \
        curl \
        tar \
        util-linux \
        libssl-dev \
        unixodbc \
        unixodbc-dev && \
    rm -rf /var/lib/apt/lists/*

FROM ubuntu:25.10

COPY --from=builder / /
