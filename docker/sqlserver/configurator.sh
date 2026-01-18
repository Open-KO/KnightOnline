#!/bin/sh
set -ae

# Check a connection to the master db
/opt/mssql-tools18/bin/sqlcmd -S sqlserver -U sa -P ${MSSQL_SA_PASSWORD} -C -d master
        
echo "All done!"