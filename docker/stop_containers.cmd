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

:: Stop the containers without removing any images or volumes
:: resume with resume_containers.sh
docker compose down

:: return to original directory
popd
