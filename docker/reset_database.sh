# Linux script

# Find script location on FS and use that as the working directory
# $0 is the command line input (e.g., ./myscript.sh, /usr/bin/myscript)
SCRIPT_PATH="$0"

# Resolve symlink
# readlink should be available to all POSIX systems
if [ -h "$SCRIPT_PATH" ] && command -v readlink >/dev/null 2>&1 ; then
    # Get symlink target
    LINK_TARGET=$(readlink "$SCRIPT_PATH")

    if [ "${LINK_TARGET#/}" != "$LINK_TARGET" ]; then
        # Absolute path
        SCRIPT_PATH="$LINK_TARGET"
    else
        # Relative path
        LINK_DIR=$(dirname "$SCRIPT_PATH")
        SCRIPT_PATH="$LINK_DIR/$LINK_TARGET"
    fi
fi
SCRIPT_DIR=$(cd "$(dirname "$SCRIPT_PATH")" && pwd)

# try to change to script directory
cd "$SCRIPT_DIR" || { echo "Failed to change to directory: $SCRIPT_DIR"; exit 1; }
# pop up a directory to be at the project root (where the docker-compose.yaml is)
cd ..
echo "Working dir: " && pwd

# Performs a clean import of the OpenKO database.  This will 
# remove any existing OpenKO database instance matching the 
# DB_NAME from the .env file (default: KN_online), then 
# create a replacement database using the latest data 
# from the OpenKO-db project.
docker exec knightonline-kodb-util-1 /usr/local/bin/cleanImport.sh
