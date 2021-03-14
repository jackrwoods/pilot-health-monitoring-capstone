echo "NOTE: This must be run with root privileges."

export NODE_VER=14.15.1
if ! node --version | grep -q ${NODE_VER}; then
	(cat /proc/cpuinfo | grep -q "Pi Zero") && if [ ! -d node-v${NODE_VER}-linux-armv6l ]; then
		echo "Installing nodejs ${NODE_VER} for armv6 from unofficial builds..."
		curl -O https://unofficial-builds.nodejs.org/download/release/v${NODE_VER}/node-v${NODE_VER}-linux-armv6l.tar.xz
		tar -xf node-v${NODE_VER}-linux-armv6l.tar.xz
	fi
	echo "Adding node to the PATH"
	PATH=$(pwd)/node-v${NODE_VER}-linux-armv6l/bin:${PATH}
fi

#echo "Building React app..."
#npm install
#npm run build

echo "Starting http server..."
#npm install -g serve
serve -s build -p 80

