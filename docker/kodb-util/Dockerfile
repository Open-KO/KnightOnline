FROM golang:1.24-alpine AS base

# Install git
RUN apk add --no-cache git ca-certificates

# Install envsubst (.env file variable substitution tool)
RUN apk add --no-cache gettext

# clone and build kodb-util so that it is available to the entry script
WORKDIR /var/lib/app
RUN git --version
RUN git clone https://github.com/Open-KO/kodb-util.git
WORKDIR /var/lib/app/kodb-util
RUN git pull
RUN git submodule update --init --recursive --remote
RUN go mod download
RUN set -e; \
    go build || (echo "Failed to build the kodb-util tool" && exit 1)

# copy tool config template
COPY kodb-util-config.yaml /var/lib/app/kodb-util/kodb-util-config.yaml.template

# copy the entry script onto the image
COPY cleanImport.sh /usr/local/bin/cleanImport.sh
RUN chmod +x /usr/local/bin/cleanImport.sh
# Remove Windows carriage returns that may break the shebang
RUN sed -i 's/\r$//' /usr/local/bin/cleanImport.sh

# keep the container alive so that it may recieve commands
# via `docker exec`
CMD ["sh", "-c", "while :; do sleep 1d; done"]
