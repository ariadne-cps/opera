

# Opera

[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT) [![Unix Status](https://github.com/ariadne-cps/opera/workflows/Unix/badge.svg)](https://github.com/ariadne-cps/opera/actions/workflows/unix.yml)
 [![Windows Status](https://github.com/ariadne-cps/opera/workflows/Windows/badge.svg)](https://github.com/ariadne-cps/opera/actions/workflows/windows.yml)
[![codecov](https://codecov.io/gh/ariadne-cps/opera/branch/main/graph/badge.svg)](https://codecov.io/gh/ariadne-cps/opera)

Opera is a tool for run-time verification of human-robot interaction, aimed at identifying future collisions based on the dynamically stored history of robots behavior.

### Building

To build the library from sources in a clean way, it is preferable that you set up a build subdirectory, say:

```
$ mkdir build && cd build
```

Then you can prepare the build environment, choosing a Release build for maximum performance:

```
$ cmake .. -DCMAKE_BUILD_TYPE=Release
```

At this point, if no error arises, you can build with:

```
$ cmake --build .
```

## Contribution guidelines ##

If you would like to contribute to Opera, please contact the developer: 

* Luca Geretti <luca.geretti@univr.it>
