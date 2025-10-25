# Newton Fractal Visualization (ISPC + C++)
Jetbrains internship test project.

This project visualizes the **Newton Fractal** for the equation:

f(z) = z^n - 1 = 0

using **ISPC** (Intel SPMD Program Compiler) for parallel computation and **C++** for orchestration, image output, and benchmarking.

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
| `--csv <path>` | Append benchmark results to a CSV file | — |
| `--no-write` | Skip image writing (for clean benchmarking) | — |
| `-h`, `--help` | Show help message | — |


