#!/bin/sh
set -a

# Check access to the service is working
/opt/mssql-tools18/bin/sqlcmd -S sqlserver -U sa -P ${MSSQL_SA_PASSWORD} -C -Q 'SELECT 1' || exit 1