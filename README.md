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
This work is built with [ARM GCC](https://developer.arm.com/Tools%20and%20Software/GNU%20Toolchain) and run with [QEMU](https://www.qemu.org/).
* macOS: `brew tap ArmMbed/homebrew-formulae && brew install arm-none-eabi-gcc qemu`
* Ubuntu Linux / Debian: `sudo apt install gcc-arm-none-eabi qemu-system-arm`

## License
`raycaster` is released under the MIT License.
Use of this source code is governed by a MIT license that can be found in the LICENSE file.
