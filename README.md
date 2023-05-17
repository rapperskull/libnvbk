# libnvbk

A library to manipulate NVBK files found on OPPO/Realme/OnePlus devices, born to avoid writing a quick and dirty hack for one of my projects.

Still a WIP: very few functions are present, mainly addressing my immediate needs.

I plan to expand it in the future, adding functions as requested. Feel free to make PRs, but keep in mind that not everything will be merged.

## Documentation

Coming soon

## Compiling

Any system with CMake should do (Visual Studio on Windows). The library has been written with portability in mind, but so far it has been tested only on Windows, Linux and Android.

macOS and BSD support is in the plans.

## License

Unless otherwise specified, all the files in this repository are licensed under the [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0).

The full text of the license can be found in [LICENSE](LICENSE).

## Third-party modules

All third party code is untouched and included as a git submodule.

### Mbed TLS

Found under `mbedtls`. Distributed under Apache License 2.0.

### portable_endian.h

Found under `include/panzi`. Public Domain.
