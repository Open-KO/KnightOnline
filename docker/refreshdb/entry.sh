#!/bin/sh
set -e

# Path to the marker file inside the persistent volume
MARKER="/var/lib/app/.first_run_done"

if [ ! -f "$MARKER" ]; then
    echo "Populating database..."
    cd /var/lib/app/kodb-util
    go run kodb-util.go -clean -import -dbuser sa -dbpass ${Sa_Password}

    # Create the marker so future runs skip this block
    touch "$MARKER"
    echo "Database populated."
else
    echo "Database already populated.  Rebuild containers for a fresh database."
fi
