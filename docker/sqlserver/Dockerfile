# SQL Server 2022 base image
FROM mcr.microsoft.com/mssql/server:2022-latest AS base

# Set SQL Server image environment variables
ENV ACCEPT_EULA=Y
ENV MSSQL_PID=Developer

USER root
WORKDIR /var/opt/appscripts

# copy the healthcheck script (checks service availability)
COPY healthcheck.sh .
RUN chmod +x healthcheck.sh

# Remove Windows carriage returns that may break the shebang
RUN sed -i 's/\r$//' healthcheck.sh

# copy the configurator script (checks master connection)
COPY configurator.sh .
RUN chmod +x configurator.sh
# Remove Windows carriage returns that may break the shebang
RUN sed -i 's/\r$//' configurator.sh

# Do not specify an entry point. We want to use the one from the base image
