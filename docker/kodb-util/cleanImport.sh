#!/bin/sh
set -e

echo "Installing a clean, up-to-date OpenKO database..."
cd /var/lib/app/kodb-util

# Compile config using .env variables
envsubst < kodb-util-config.yaml.template > kodb-util-config.yaml

# Update the repo/submodules
git pull
git submodule update --recursive --remote
go mod download
go run kodb-util.go -clean -import -dbuser sa -dbpass ${MSSQL_SA_PASSWORD}
