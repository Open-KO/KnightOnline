# SQL Server 2022 base image
FROM mcr.microsoft.com/mssql/server:2022-latest AS base

# Set SQL Server image environment variables
ENV ACCEPT_EULA=Y
ENV MSSQL_PID=Developer

# copy the healthcheck script (checks service availability)
COPY --chmod=0755 healthcheck.sh /usr/local/bin/healthcheck.sh

# copy the configurator script (checks master connection)
COPY --chmod=0755 configurator.sh /usr/local/bin/configurator.sh

# Do not specify an entry point. We want to use the one from the base image
