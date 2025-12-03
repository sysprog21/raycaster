# raycaster
An efficient and lean implementation of the [ray casting](https://en.wikipedia.org/wiki/Ray_casting) algorithm.

## Features
- no operating-system required
- no floating-point operations
- no division operations
- UART keyboard input
- 8 x 8-bit multiplications per vertical line
- precalculated trigonometric and perspective tables

## Prerequisites

### SDL2 (Native)
* macOS: `brew install sdl2`
* Ubuntu Linux / Debian: `sudo apt install libsdl2-dev`
* Arch Linux: `sudo pacman -S sdl2`

### ARM Baremetal
Built with [ARM GCC](https://developer.arm.com/Tools%20and%20Software/GNU%20Toolchain) and run with [QEMU](https://www.qemu.org/).
* macOS: `brew install arm-none-eabi-gcc qemu`
* Ubuntu Linux / Debian: `sudo apt install gcc-arm-none-eabi qemu-system-arm`
* Arch Linux: `sudo pacman -S arm-none-eabi-gcc arm-none-eabi-newlib qemu`

## Build and Run

```bash
# Build and run SDL2 version (default)
make
./raycaster_sdl

# Build ARM baremetal version
make arm

# Build with custom cross-compiler
CROSS_COMPILE=arm-linux-gnueabihf- make arm

# Run in QEMU (builds if needed)
make baremetal
```

### Controls
- SDL2: Arrow keys to move/rotate, ESC to quit
- Baremetal (UART): `w`/`s` to move, `a`/`d` to rotate

## License
`raycaster` is released under the MIT License.
Use of this source code is governed by a MIT license that can be found in the LICENSE file.
