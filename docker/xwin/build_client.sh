set -euox pipefail
export DOCKER_DEFAULT_PLATFORM=linux/amd64
export IMAGE=cross-win-builder

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

# Check if the image is already built
if ! docker image inspect $IMAGE >/dev/null 2>&1; then
    # Image not found – build it from the Dockerfile in the current directory
    echo "Building '$IMAGE' image"
    docker build -t $IMAGE .
fi

# try to change to script directory
cd "$SCRIPT_DIR" || { echo "Failed to change to directory: $SCRIPT_DIR"; exit 1; }
# pop up a directory to be at the project root (where the docker-compose.yaml is)
cd ../..
echo "Working dir: " && pwd

HOST_GIT_DIR=$(pwd)
BUILD_ARTIFACTS="$(pwd)/cmake-build-debug"
mkdir -p "$BUILD_ARTIFACTS"

# remove any stray containers
CONTAINER_NAME="winbuild-temp"
OLD_CID=$(docker ps -aq --filter "name=${CONTAINER_NAME}")
if [[ -n "$OLD_CID" ]]; then
    docker stop "$OLD_CID" 2>/dev/null || true
    docker rm "$OLD_CID"
    echo "Container \"$CONTAINER_NAME\" ($OLD_CID) removed."
fi

containerID=$(docker create \
            --name "${CONTAINER_NAME}" \
            -e CONTAINER_SRC_DIR=/src \
            -e BUILD_DIR=/tmp/build \
            -v "${HOST_GIT_DIR}":/src:ro \
            "$IMAGE" tail -f /dev/null)

echo "winbuild-temp container starting..."
docker start "$containerID"
until docker inspect --format='{{.State.Running}}' "$containerID" | grep -q true; do
    sleep 0.1
done

# Make sure the container is removed regardless of error/success cases
defer() {
  echo "Cleaning up container $containerID"
  #docker stop "$containerID" 2>/dev/null || true
  #docker rm "$containerID"
}
trap defer EXIT

docker exec "$containerID" bash -c "$(cat <<'SCRIPT'
# IN-CONTAINER SCRIPT
set -euox pipefail

# Verify the compiler is present
command -v x86_64-w64-mingw32-g++ >/dev/null || { echo "Missing MinGW compiler"; exit 1; }

if [[ ! -f ${CONTAINER_SRC_DIR}/CMakeLists.txt ]]; then
  echo "Error: ${CONTAINER_SRC_DIR}/CMakeLists.txt was not found in the mounted source." >&2
  exit 1
fi

mkdir -p "${BUILD_DIR}"

cmake -S "${CONTAINER_SRC_DIR}" \
      -B "${BUILD_DIR}" \
      -G "Ninja" \
      -DCMAKE_C_COMPILER="x86_64-w64-mingw32-gcc" \
      -DCMAKE_CXX_COMPILER="x86_64-w64-mingw32-g++" \
      -DCMAKE_SYSTEM_NAME="Windows" \
      -DLLFIO_ASSUME_CROSS_COMPILING=ON \
      -DODBC_LIBRARY=/usr/lib/x86_64-linux-gnu/libodbc.so \
      -DODBC_INCLUDE_DIR=/usr/include \
      -DOPENKO_BUILD_CLIENT_TOOLS=OFF \
      -DOPENKO_BUILD_TOOLS=OFF \
      -DOPENKO_BUILD_SERVERS=OFF \
      -DCMAKE_VERBOSE_MAKEFILE=ON \
      -Wno-dev

      # 32-bit
      #      -DCMAKE_C_FLAGS="-m32" \
      #      -DCMAKE_CXX_FLAGS="-m32" \
      #      -DCMAKE_C_COMPILER="i686-w64-mingw32-gcc" \
      #      -DCMAKE_CXX_COMPILER="i686-w64-mingw32-g++" \

cmake --build "${BUILD_DIR}" \
      --target WarFare \
      --config Debug \
      --verbose \
      -j 14

# END IN-CONTAINER SCRIPT
SCRIPT
)"

docker cp "$containerID:/tmp/build/bin" "${BUILD_ARTIFACTS}/bin"

echo "Windows executable built:"
ls -l "$BUILD_ARTIFACTS/bin/Debug/*.exe"
