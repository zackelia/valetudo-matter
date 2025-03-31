# valetudo-matter

Matter component for Valetudo

> [!WARNING]
> This is early-on development grade software!

## About

valetudo-matter exposes a Valetudo device on your network as a Matter device. It implements a (naive albeit good enough) MQTT server for Valetudo to connect to and will communicate between Matter controllers and Valetudo. I've currently tested it as a bridge on separate hardware but the goal would be to run it on the Valetudo hardware itself to have one cohesive unit.

It is implemented as a separate C++ component for a few reasons:
* Valetudo is not open to contributions, let alone Matter
* I'm not willing to maintain a fork of Valetudo
* I have no interest in writing TypeScript/JavaScript
* The Matter Rust SDK is not mature enough yet

## Compatibility

As of the time of writing, I have conducted the following testing:
* Matter's `chip-tool` - this is where I have conducted nearly all of my testing of what I *think* other Matter controllers will do
* Home Assistant - minimal testing as Matter 1.4 (includes room selection) is not supported yet for robot vacuum cleaners (RVCs)
* Apple Home - minimal testing, but so far so good, it has Matter 1.4!

## Development

Personal notes on building/running this project before I formalize it better.

### prep repos

```
git submodule update --init
cd third_party/connectedhomeip
git submodule update --init --recursive -- \
    third_party/pigweed \
    third_party/jsoncpp \
    third_party/nlassert \
    third_party/nlio
source scripts/activate.sh
```

### zap

```
./third_party/connectedhomeip/scripts/tools/zap/run_zaptool.sh
python3 third_party/connectedhomeip/scripts/tools/zap/generate.py /workspaces/valetudo-matter/zap/valetudo-matter.zap -z /workspaces/valetudo-matter/third_party/connectedhomeip/src/app/zap-templates/zcl/zcl.json
```

### build

```
gn gen out/host
ninja -C out/host
./out/host/valetudo-matter
```

### chip tool
```
cd third_party/connectedhomeip
git submodule update --init --recursive -- \
    third_party/editline \
    third_party/libwebsockets
git clone https://github.com/open-source-parsers/jsoncpp -b 1.9.5 --depth 1 third_party/jsoncpp/repo
cd examples/chip-tool
gn gen out/host
ninja -C out host
./out/host/chip-tool -h
```

```
out/host/chip-tool pairing onnetwork 1 20202021
out/host/chip-tool onoff toggle 1 1  # ID 1, endpoint 1 (RVC)
out/host/chip-tool pairing unpair
```

### socat listener

socat tcp-listen:1884,reuseaddr,fork tcp:localhost:1883

### valetudo config

lots of data doesn't load if you are sitting on the mqtt settings screen
