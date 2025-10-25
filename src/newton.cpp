#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <stdexcept>
#include <limits>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <cstdint>
#include <cstdio>

#include "newtonApprox.h"
#include "lodepng.h"

static constexpr int    DEF_POWER     = 3;
static constexpr size_t DEF_WIDTH     = 10000;
static constexpr size_t DEF_HEIGHT    = 10000;
static constexpr unsigned short DEF_MAX_ITER = 25;
static constexpr double DEF_MIN_STEP2 = 1e-6;

typedef struct Points 
{
    std::vector<double> re;
    std::vector<double> im;
    std::vector<short> approaches;
    std::vector<float> convSpeed;

    Points(size_t width, size_t height){
        re.resize(width*height);
        im.resize(width*height);
        approaches.resize(width*height);
        convSpeed.resize(width*height);
    }
} Points;

typedef struct Roots{
    std::vector<double> reRoots;
    std::vector<double> imRoots;

    Roots(short power){
        reRoots.resize(power);
        imRoots.resize(power);
    }
} Roots;

typedef struct FrameBuff{
    size_t width;
    size_t height;

    std::vector<unsigned char> red;
    std::vector<unsigned char> green;
    std::vector<unsigned char> blue;

    FrameBuff(size_t width, size_t height){
        this->width = width;
        this->height = height;
        red.resize(width*height);
        green.resize(width*height);
        blue.resize(width*height);
    }
} FrameBuff;

/* ---------- I/O helpers ---------- */

void writePPM(const FrameBuff &fb, const std::string &filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out) throw std::runtime_error("Cannot open " + filename);

    out << "P6\n" << fb.width << " " << fb.height << "\n255\n";

    for (size_t y = 0; y < fb.height; ++y) {
        const size_t base = y * fb.width;
        for (size_t x = 0; x < fb.width; ++x) {
            const size_t i = base + x;
            out.put (static_cast<char>(fb.red[i]));
            out.put(static_cast<char>(fb.green[i]));
            out.put(static_cast<char>(fb.blue[i]));
        }
    }
}

void writePNG(const FrameBuff &fb, const std::string &filename) {
    const size_t N = fb.width * fb.height;
    std::vector<unsigned char> interleaved(3 * N);

    for (size_t i = 0; i < N; ++i) {
        interleaved[3*i + 0] = fb.red[i];
        interleaved[3*i + 1] = fb.green[i];
        interleaved[3*i + 2] = fb.blue[i];
    }

    // encode to PNG (RGB, 8-bit)
    unsigned err = lodepng::encode(filename, interleaved, fb.width, fb.height, LCT_RGB);
    if (err) {
        throw std::runtime_error("PNG encode error " + std::to_string(err) +
                                 ": " + lodepng_error_text(err));
    }
}

/* ---------- CLI help ---------- */

static void print_help(const char* prog) {
    std::cout <<
R"(Usage:
  )" << prog << R"( [options]

Options:
  -p, --power <int>         Polynomial power (roots on unit circle). Default: )" << DEF_POWER << R"(
  -W, --width <int>         Image width in pixels.              Default: )" << DEF_WIDTH << R"(
  -H, --height <int>        Image height in pixels.             Default: )" << DEF_HEIGHT << R"(
  -i, --max-iter <int>      Max Newton iterations per pixel.    Default: )" << DEF_MAX_ITER << R"(
  -m, --min-step <float>    Convergence threshold (squared).    Default: )" << DEF_MIN_STEP2 << R"(

  -o, --output <path>       Output filename. Default: derived from format (NEWTON.png or NEWTON.ppm)
      --png                 Write PNG (via lodepng).            (default)
      --ppm                 Write PPM (P6, binary)

  # Benchmarking
      --bench <runs>        Enable benchmarking with <runs> timed runs
      --warmup <n>          Warmup runs (not timed). Default: 1
      --csv <path>          Append results to CSV at <path>
      --no-write            Skip writing image (recommended for clean timings)

  -h, --help                Show this help and exit.

Examples:
  )" << prog << R"( --png -p 5 -W 800 -H 600 -i 50 -m 1e-8 -o out.png
  )" << prog << R"( --ppm --power 7 --width 4096 --height 4096
  )" << prog << R"( --bench 10 --warmup 2 --no-write --csv results.csv -W 8000 -H 8000
)";
}

/* ---------- Parse helpers ---------- */

static bool parseInt(const std::string& s, long long& out) {
    try {
        size_t idx = 0;
        long long v = std::stoll(s, &idx, 10);
        if (idx != s.size()) return false;
        out = v;
        return true;
    } catch (...) { return false; }
}
static bool parseDouble(const std::string& s, double& out) {
    try {
        size_t idx = 0;
        double v = std::stod(s, &idx);
        if (idx != s.size()) return false;
        out = v;
        return true;
    } catch (...) { return false; }
}

/* ---------- Benchmark utilities ---------- */

// simple 64-bit FNV-1a checksum over the RGB buffers
static inline uint64_t fnv1a64_rgb(const FrameBuff& fb) {
    const uint64_t FNV_OFFSET = 1469598103934665603ull;
    const uint64_t FNV_PRIME  = 1099511628211ull;
    uint64_t h = FNV_OFFSET;
    const size_t N = fb.width * fb.height;
    for (size_t i = 0; i < N; ++i) {
        h ^= fb.red[i];   h *= FNV_PRIME;
        h ^= fb.green[i]; h *= FNV_PRIME;
        h ^= fb.blue[i];  h *= FNV_PRIME;
    }
    return h;
}

struct Stats {
    double min_ms = 0;
    double mean_ms = 0;
    double median_ms = 0;
    double stddev_ms = 0;
    double mpix_per_s = 0;
};

static Stats compute_stats(const std::vector<double>& ms, size_t pixels) {
    Stats s;
    if (ms.empty()) return s;
    s.min_ms = *std::min_element(ms.begin(), ms.end());
    s.mean_ms = std::accumulate(ms.begin(), ms.end(), 0.0) / ms.size();
    std::vector<double> sorted = ms;
    std::sort(sorted.begin(), sorted.end());
    if (sorted.size() & 1) s.median_ms = sorted[sorted.size()/2];
    else s.median_ms = 0.5 * (sorted[sorted.size()/2 - 1] + sorted[sorted.size()/2]);
    // stddev
    double acc = 0.0;
    for (double v : ms) { double d = v - s.mean_ms; acc += d*d; }
    s.stddev_ms = std::sqrt(acc / ms.size());
    s.mpix_per_s = (pixels / 1e6) / (s.mean_ms / 1000.0);
    return s;
}

static void maybe_write_csv(const std::string& path,
                            const Stats& s,
                            size_t width, size_t height,
                            int power, unsigned short max_iter,
                            double min_step2,
                            uint64_t checksum,
                            int runs, int warmup) {
    if (path.empty()) return;

    bool need_header = false;
    {
        std::ifstream test(path);
        if (!test.good()) need_header = true;
    }
    std::ofstream out(path, std::ios::app);
    if (!out) {
        std::cerr << "Warning: can't open CSV '" << path << "' for append\n";
        return;
    }
    if (need_header) {
        out << "width,height,power,max_iter,min_step2,runs,warmup,checksum,"
               "min_ms,mean_ms,median_ms,stddev_ms,mpix_per_s\n";
    }
    out << width << "," << height << "," << power << "," << max_iter << ","
        << min_step2 << "," << runs << "," << warmup << ","
        << checksum << ","
        << s.min_ms << "," << s.mean_ms << "," << s.median_ms << ","
        << s.stddev_ms << "," << s.mpix_per_s << "\n";
}

/* ---------- Main ---------- */

int main(int argc, char** argv) {
    // Defaults
    int power = DEF_POWER;
    size_t width = DEF_WIDTH;
    size_t height = DEF_HEIGHT;
    unsigned short max_iter = DEF_MAX_ITER;
    double min_step2 = DEF_MIN_STEP2;

    enum class Format { PNG, PPM };
    Format fmt = Format::PNG;
    std::string out_path; // if empty, choose by fmt

    // Benchmark options
    int bench_runs = 0;   // 0 = disabled
    int warmup_runs = 1;
    std::string csv_path;
    bool no_write = false;

    for (int a = 1; a < argc; ++a) {
        std::string arg = argv[a];

        auto lastParam = [&](const char* name) {
            if (a + 1 >= argc) {
                std::cerr << "Missing value for " << name << "\n";
                print_help(argv[0]);
                return false;
            }
            return true;
        };

        if (arg == "-h" || arg == "--help") {
            print_help(argv[0]);
            return 0;
        } else if (arg == "-p" || arg == "--power") {
            if (!lastParam(arg.c_str())) return 1;
            long long v;
            if (!parseInt(argv[++a], v) || v < 1 || v > std::numeric_limits<short>::max()) {
                std::cerr << "Invalid --power: " << argv[a] << "\n";
                return 1;
            }
            power = static_cast<int>(v);
        } else if (arg == "-W" || arg == "--width") {
            if (!lastParam(arg.c_str())) return 1;
            long long v;
            if (!parseInt(argv[++a], v) || v < 1) {
                std::cerr << "Invalid --width: " << argv[a] << "\n";
                return 1;
            }
            width = static_cast<size_t>(v);
        } else if (arg == "-H" || arg == "--height") {
            if (!lastParam(arg.c_str())) return 1;
            long long v;
            if (!parseInt(argv[++a], v) || v < 1) {
                std::cerr << "Invalid --height: " << argv[a] << "\n";
                return 1;
            }
            height = static_cast<size_t>(v);
        } else if (arg == "-i" || arg == "--max-iter") {
            if (!lastParam(arg.c_str())) return 1;
            long long v;
            if (!parseInt(argv[++a], v) || v < 1 || v > std::numeric_limits<unsigned short>::max()) {
                std::cerr << "Invalid --max-iter: " << argv[a] << "\n";
                return 1;
            }
            max_iter = static_cast<unsigned short>(v);
        } else if (arg == "-m" || arg == "--min-step") {
            if (!lastParam(arg.c_str())) return 1;
            double v;
            if (!parseDouble(argv[++a], v) || v <= 0.0) {
                std::cerr << "Invalid --min-step: " << argv[a] << "\n";
                return 1;
            }
            min_step2 = v;
        } else if (arg == "-o" || arg == "--output") {
            if (!lastParam(arg.c_str())) return 1;
            out_path = argv[++a];
        } else if (arg == "--png") {
            fmt = Format::PNG;
        } else if (arg == "--ppm") {
            fmt = Format::PPM;
        } else if (arg == "--bench") {
            if (!lastParam(arg.c_str())) return 1;
            long long v;
            if (!parseInt(argv[++a], v) || v < 1 || v > 100000) {
                std::cerr << "Invalid --bench: " << argv[a] << "\n";
                return 1;
            }
            bench_runs = static_cast<int>(v);
        } else if (arg == "--warmup") {
            if (!lastParam(arg.c_str())) return 1;
            long long v;
            if (!parseInt(argv[++a], v) || v < 0 || v > 1000) {
                std::cerr << "Invalid --warmup: " << argv[a] << "\n";
                return 1;
            }
            warmup_runs = static_cast<int>(v);
        } else if (arg == "--csv") {
            if (!lastParam(arg.c_str())) return 1;
            csv_path = argv[++a];
        } else if (arg == "--no-write") {
            no_write = true;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            print_help(argv[0]);
            return 1;
        }
    }

    if (out_path.empty()) {
        out_path = (fmt == Format::PNG) ? "NEWTON.png" : "NEWTON.ppm";
    }

    Points points(width, height);
    Roots roots(static_cast<short>(power));
    FrameBuff buff(width, height);

    const size_t pixels = width * height;

    auto run_once = [&]() {
        ispc::approxISPC(width, height,
                         roots.reRoots.data(), roots.imRoots.data(),
                         static_cast<unsigned short>(power),
                         points.re.data(), points.im.data(),
                         buff.red.data(), buff.green.data(), buff.blue.data(),
                         max_iter, min_step2);
    };

    // --- Benchmarking mode ---
    if (bench_runs > 0) {
        // Warmup
        for (int w = 0; w < warmup_runs; ++w) run_once();

        std::vector<double> times_ms;
        times_ms.reserve(static_cast<size_t>(bench_runs));
        uint64_t checksum = 0;

        for (int r = 0; r < bench_runs; ++r) {
            auto t0 = std::chrono::steady_clock::now();
            run_once();
            auto t1 = std::chrono::steady_clock::now();
            std::chrono::duration<double, std::milli> dt = t1 - t0;
            times_ms.push_back(dt.count());
            // compute checksum each run (cheap)
            checksum ^= fnv1a64_rgb(buff) + 0x9e3779b97f4a7c15ull + (uint64_t)r + (checksum<<6) + (checksum>>2);
        }

        Stats s = compute_stats(times_ms, pixels);

        std::cout << "Benchmark results (" << bench_runs << " runs"
                  << ", warmup=" << warmup_runs << ")\n";
        std::cout << "  Size: " << width << "x" << height
                  << "  Pixels: " << pixels << "\n";
        std::cout << "  Power: " << power
                  << "  MaxIter: " << max_iter
                  << "  MinStep2: " << min_step2 << "\n";
        std::cout << "  min:    " << s.min_ms    << " ms\n";
        std::cout << "  mean:   " << s.mean_ms   << " ms\n";
        std::cout << "  median: " << s.median_ms << " ms\n";
        std::cout << "  stddev: " << s.stddev_ms << " ms\n";
        std::cout << "  thruput: " << s.mpix_per_s << " MPix/s\n";
        std::cout << "  checksum: 0x" << std::hex << checksum << std::dec << "\n";

        maybe_write_csv(csv_path, s, width, height, power, max_iter, min_step2, checksum, bench_runs, warmup_runs);

        // Optionally write the image even in bench mode
        if (!no_write) {
            try {
                if (fmt == Format::PNG) writePNG(buff, out_path);
                else writePPM(buff, out_path);
            } catch (const std::exception& e) {
                std::cerr << "Write error: " << e.what() << "\n";
                return 1;
            }
        }
        return 0;
    }

    // --- Normal (non-benchmark) mode ---
    ispc::approxISPC(width, height,
                      roots.reRoots.data(), roots.imRoots.data(),
                      static_cast<unsigned short>(power),
                      points.re.data(), points.im.data(),
                      buff.red.data(), buff.green.data(), buff.blue.data(),
                      max_iter, min_step2);

    try {
        if (fmt == Format::PNG) {
            writePNG(buff, out_path);
        } else {
            writePPM(buff, out_path);
        }
    } catch (const std::exception& e) {
        std::cerr << "Write error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
