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
sudo ./update.sh

```

## Usage

Communication between the server and a client goes via UDP.
Clients can connect and as long as at least one client is connected, 
motion will stream the image on port 8081

Command | Meaning | Possible answers
--------|---------|--------
0xACDC | Connect to the server | 0 - Connection successfull
-|-|1 - Server is full
-|-|2 - Already connected
0xDCAC | Disconnect from the server | -
0xAAAA | Disconnect all clients | -
