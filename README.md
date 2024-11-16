# development

## prep repos

```
git submodule update --init
cd third_party/connectedhomeip
git submodule update --init --recursive --remote -- third_party/pigweed/
git submodule update --init --recursive --remote -- third_party/jsoncpp/
git clone https://android.googlesource.com/platform/external/perfetto -b v39.0 --depth 1 third_party/perfetto/repo
git submodule update --init --recursive --remote -- third_party/nlunit-test/
git submodule update --init --recursive --remote -- third_party/nlassert/
git submodule update --init --recursive --remote -- third_party/nlio/
source scripts/activate.sh
```

## zap

```
./third_party/connectedhomeip/scripts/tools/zap/run_zaptool.sh
python3 third_party/connectedhomeip/scripts/tools/zap/generate.py /workspaces/valetudo-matter/zap/valetudo-matter.zap -z /workspaces/valetudo-matter/third_party/connectedhomeip/src/app/zap-templates/zcl/zcl.json
```

## build

```
gn gen out/host
ninja -C out/host
```

## chip tool
```
cd third_party/connectedhomeip
git submodule update --init --recursive --remote -- third_party/editline/
git submodule update --init --recursive --remote -- third_party/libwebsockets/
git clone https://github.com/open-source-parsers/jsoncpp -b 1.9.5 --depth 1 third_party/jsoncpp/repo
cd examples/chip-tool
gn gen out/host
ninja -C out host
./out/host/chip-tool -h
```
```
out/host/chip-tool pairing onnetwork 1 20202021
out/host/chip-tool onoff toggle 1 1  # ID 1, endpoint 1 (RVC)
out/host/chip-tool unpair
```
