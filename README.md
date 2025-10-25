# Newton Fractal Visualization (ISPC + C++)
Jetbrains internship test project.

This project is a CLI application that renders the [**Newton Fractal**](https://en.wikipedia.org/wiki/Newton_fractal) for the equation:

f(z) = z^n - 1 = 0 to PNG.

As a backend it uses **ISPC** (Intel SPMD Program Compiler) for parallel computation and **C++** for orchestration, image output, and benchmarking.

The resulting image shows which **root** each initial complex value converges to (color), and how **quickly** it converges (brightness).

## Examples:

![Example Newton Fractal (3 power)](Examples/NEWTON3.png)
![Example Newton Fractal (4 power)](Examples/NEWTON4.png)
![Example Newton Fractal (5 power)](Examples/NEWTON5.png)
---

## ⚙️ Command-Line Options

| Option | Description | Default |
|--------|--------------|----------|
| `-p`, `--power <int>` | Power of the polynomial n in z^n - 1 = 0 | `3` |
| `-W`, `--width <int>` | Image width (pixels) | `10000` |
| `-H`, `--height <int>` | Image height (pixels) | `10000` |
| `-i`, `--max-iter <int>` | Maximum Newton iterations | `25` |
| `-m`, `--min-step <float>` | Convergence threshold (squared) | `1e-6` |
| `-o`, `--output <path>` | Output file path | `NEWTON.png` |
| `--png` | Output PNG (default) | — |
| `--ppm` | Output PPM | — |
| `--bench <runs>` | Run benchmark mode with given number of runs | — |
| `--warmup <n>` | Warm-up runs before timing | `1` |
| `--no-write` | Skip image writing (for clean benchmarking) | — |
| `-h`, `--help` | Show help message | — |


## Dependencies:
- **C++17 (g++)** or later 
- **ISPC** compiler 
- **Make** (for building)

## Build instructions:
```bash
make
```
---
## Example usages:
```bash
./newton --png -p 5 -W 2000 -H 2000 -i 40 -m 1e-8 -o fractal.png

./newton --bench 10 --warmup 2 --no-write -W 8000 -H 8000
```
## Shoutout 
to [lodepng](https://lodev.org/lodepng/), compact png encoder that I`ve used for output.


---

## Note experimental branch

On experemental branch lies imgui based GUI application, that tries to render fractal in real time. It allows also to change parameters and see the results live.

To try it out clone the branch and build. Its unpolihed untested and could (**mostprobably**) contain bugs but its still funny to play with the fractal live by changing the parameters. 

```bash
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release  
cmake --build build -j  
/build/gui_newton
```
