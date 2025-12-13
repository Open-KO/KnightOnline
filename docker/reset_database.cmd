:: Windows script
:: Find script location on FS and use that as the working directory
:: %~dp0: drive letter + back‑slash + directory
set "SCRIPT_DIR=%~dp0"
pushd "%SCRIPT_DIR%" || (
    echo Failed to change to directory: "%SCRIPT_DIR%"
    exit /b 1
)
cd ..
echo Working directory set to: %CD%

:: Performs a clean import of the OpenKO database.  This will 
:: remove any existing OpenKO database instance matching the 
:: DB_NAME from the .env file (default: KN_online), then 
:: create a replacement database using the latest data 
:: from the OpenKO-db project.
docker exec knightonline-kodb-util-1 /usr/local/bin/cleanImport.sh

:: return to original directory
popd
