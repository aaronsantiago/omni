#!/bin/bash
set -e

# Installer based on OBS-NDI plugin

LIBNDI_INSTALLER_NAME="Install_NDI_SDK_v5_Linux"
LIBNDI_INSTALLER="$LIBNDI_INSTALLER_NAME.tar.gz"

pushd /tmp  2>/dev/null
if [ "$?" -ne "0" ]; then
    echo "Cannot use temporary location /tmp, exitting."
    exit 1
fi

curl -L -o $LIBNDI_INSTALLER https://downloads.ndi.tv/SDK/NDI_SDK_Linux/$LIBNDI_INSTALLER -f --retry 5
tar -xf $LIBNDI_INSTALLER

yes | PAGER="cat" sh $LIBNDI_INSTALLER_NAME.sh

echo
# read -p "Using this script you agree with the NDI license listed above, please confirm with enter or cancel by pressing CTRL+C"
echo

rm -rf ./ndisdk
mv "./NDI SDK for Linux" ./ndisdk

popd 2>/dev/null

echo
echo "Download completed, please carefully read the NDI license before using it."
echo

if [ "$1" == "install" ]; then
    if [ -z "$2" ]; then
        echo -e "You've chosen to install the NDI libs but did not select the platform: \n`ls /tmp/ndisdk/lib/`"
        exit 1
    fi

    cp -P /tmp/ndisdk/lib/$2/* /usr/local/lib/
    cp -P /tmp/ndisdk/include/* /usr/local/include/
    ldconfig

    echo
    echo "Sucesfully installed libndi to /usr/local"
    echo
fi
