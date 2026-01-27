set -euo pipefail

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

# try to change to ClientData directory - we want this to be the working dir
cd "$SCRIPT_DIR" || { echo "Failed to change to directory: $SCRIPT_DIR"; exit 1; }
cd ../../cmake-build-debug/ClientData
echo "Working dir: " && pwd

# Ensure umu-launcher is installed on the host.  This allows us to
# launch the game via proton without Steam
UMU_BIN=$(which umu-run 2>/dev/null || true)
if [[ -z "$UMU_BIN" ]]; then
    echo "Could not find umu-run.  Install umu-launcher via your package manager." >&2
    exit 1
fi

# Verify that the client executable exists
# We copy it to ClientData before running via umu-launcher so that we
# don't have to deal with working directories (this is all going to be running
# inside a mounted wine prefix)
WARFARE_EXE_BIN=$(realpath "../bin/Debug/KnightOnLine.exe")
WARFARE_EXE="KnightOnLine.exe"
if [[ ! -f $WARFARE_EXE_BIN ]]; then
    echo "Client executable not found in bin/Debug: $WARFARE_EXE_BIN" >&2
    exit 1
fi
echo "Copying client executable from bin/Debug..."
cp "$WARFARE_EXE_BIN" .
echo "Using WarFare executable: $WARFARE_EXE"

export GAMEID=openko
export LOG_DIR=openko
export PROTONFIXES_DISABLE=1
export PROTONPATH=GE-Proton Latest
export WINEPREFIX="${HOME}/.steam/compatibilitytools.d/OpenKO-prefix"
umu-run $WARFARE_EXE
