# raycaster
An efficient and lean implementation of the [ray casting](https://en.wikipedia.org/wiki/Ray_casting) algorithm.

## Features
- no floating-point operations
- no division operations
- no memory allocation
- 8 x 8-bit multiplications per vertical line
- precalculated trigonometric and perspective tables

## Prerequisites
This work is built with [SDL2](https://www.libsdl.org/).
* macOS: `brew install sdl2`
* Ubuntu Linux / Debian: `sudo apt install libsdl2-dev`

## License
`raycaster` is released under the MIT License.
Use of this source code is governed by a MIT license that can be found in the LICENSE file.
