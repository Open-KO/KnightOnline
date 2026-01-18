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

:: Removes any existing sqlserver/kodb-util images/volumes, 
docker compose down --rmi all -v

:: then creates/starts new versions of them.  
docker compose up --build sqlserver -d
docker compose up --build kodb-util -d

:: Before the script completes, the script to create an up-to-date 
:: OpenKO database will be invoked.
docker exec knightonline-kodb-util-1 /usr/local/bin/cleanImport.sh

:: return to original directory
popd
