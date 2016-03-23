#!/bin/bash

# Redirect output
logfile=build.log
mkfifo ${logfile}.pipe
tee < ${logfile}.pipe $logfile &
exec &> ${logfile}.pipe
rm ${logfile}.pipe

# Set path to steam runtime sdk change this to your path!
# Arch
#export STEAM_RUNTIME_ROOT="/run/media/vincent/dbcbf69d-8162-4768-976c-c7c5b5ace72b/sourceengine/steam-runtime-sdk"
# Ubuntu
export STEAM_RUNTIME_ROOT="/media/vincent/dbcbf69d-8162-4768-976c-c7c5b5ace72b/sourceengine/steam-runtime-sdk"

# Stop the script if we run into any errors
set -e

if ! [ -d "${STEAM_RUNTIME_ROOT}" ]; then
  echo "You need to set STEAM_RUNTIME_ROOT to a valid directory in order to compile!" >&2
  exit 2
fi

# Store away the PATH variable for restoration
OLD_PATH=$PATH

# Set our host and target architectures
if [ -z "${STEAM_RUNTIME_HOST_ARCH}" ]; then
  if [ "$(uname -m)" == "i686" ]; then
    STEAM_RUNTIME_HOST_ARCH=i386
  elif [ "$(uname -m)" == "x86_64" ]; then
    STEAM_RUNTIME_HOST_ARCH=amd64
  else
    echo "Unknown target architecture: ${STEAM_RUNTIME_HOST_ARCH}"
    exit 1
  fi
fi

if [ -z "$STEAM_RUNTIME_TARGET_ARCH" ]; then
  STEAM_RUNTIME_TARGET_ARCH=$STEAM_RUNTIME_HOST_ARCH
fi

# Force 32 bit build on 64 bit
export STEAM_RUNTIME_TARGET_ARCH="i386"

echo "Host architecture set to $STEAM_RUNTIME_HOST_ARCH"
echo "Target architecture set to $STEAM_RUNTIME_TARGET_ARCH"

# Check if our runtime is valid
if [ ! -d "${STEAM_RUNTIME_ROOT}/runtime/${STEAM_RUNTIME_TARGET_ARCH}" ]; then
    echo "$0: ERROR: Couldn't find steam runtime directory" >&2
    echo "Do you need to run setup.sh to download the ${STEAM_RUNTIME_TARGET_ARCH} target?" >&2
    exit 2
fi

export PATH="${STEAM_RUNTIME_ROOT}/bin:$PATH"

echo

# Cleanup
#echo "Cleanup..."
#make -f games.mak clean
#rm -f ../game/mod_episodic/bin/*.so
#rm -f ../game/mod_episodic/bin/*.so.dbg

#echo

# Create Game Projects
echo "Create Game Projects..."
pushd `dirname $0`
devtools/bin/vpc /episodic +game /mksln games
popd

# Build TE120
echo "Using ccache."
export PATH="/usr/lib/ccache/bin/:$PATH"
echo "Building TE120..."
make -f games.mak

# Copy files
echo "Copy files..."
cp -f ../game/mod_episodic/bin/client.so "/media/vincent/dbcbf69d-8162-4768-976c-c7c5b5ace72b/SteamLibrary/steamapps/common/Transmissions Element 120/te120/bin"
cp -f ../game/mod_episodic/bin/server.so "/media/vincent/dbcbf69d-8162-4768-976c-c7c5b5ace72b/SteamLibrary/steamapps/common/Transmissions Element 120/te120/bin"

echo "Cleaning up..."
export PATH=$OLD_PATH

echo "TE120 Build Complete!"
