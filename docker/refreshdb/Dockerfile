FROM golang:1.24-alpine AS base

# Install git
RUN apk add --no-cache git ca-certificates

# clone and build kodb-util so that it is available to the entry script
WORKDIR /var/lib/app
RUN git --version | tee /dev/stdout
RUN git clone https://github.com/Open-KO/kodb-util.git
WORKDIR /var/lib/app/kodb-util
RUN git pull
RUN git submodule update --init --recursive --remote
RUN go mod download
RUN set -e; \
    go build || (echo "Failed to build the kodb-util tool" && exit 1)

# copy tool config
COPY kodb-util-config.yaml /var/lib/app/kodb-util/kodb-util-config.yaml

# copy the entry script onto the image
COPY entry.sh /usr/local/bin/entry.sh
RUN chmod +x /usr/local/bin/entry.sh
