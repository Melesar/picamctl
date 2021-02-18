# picamctl

A server program that allows to turn on the raspberry pi camera remotely.

## Installation

To control the camera, this program use the [motion](https://github.com/Motion-Project/motion) package. First, you need to install it

```bash

sudo apt install motion

```

Then download the source code and build it on the Raspberry PI

```bash

git clone https://github.com/Melesar/picamctl
cd picamctl
make pi
sudo make install

```

## Usage

When the program is running, it listens for UDP packages on port 8085. To connect as a client thus enabling the camera, send the two-bytes value `0xACDC`. This can be achieved, for example, with bash:

```bash

printf '\xac\xdc' > /dev/udp/{your-pi-ip-address}/8085

```

To disconnect, send `0xDCAC`. When no clients are connected, the camera will be automatically stopped.

