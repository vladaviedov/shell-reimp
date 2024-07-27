# mesh
![build](https://github.com/vladaviedov/mesh/actions/workflows/build.yml/badge.svg)

Unix shell implementation (incomplete) but with meta commands.

## Requirements

- C99-capable tool chain.
- GNU make.
- Yacc.
- Flex.

## Build
```
git submodule update --init
make
```

### Development

- `make release` - Build release binary (same as `make`).
- `make debug` - Build debug binary.
- `make clean` - Remove build files.
