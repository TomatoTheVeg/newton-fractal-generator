// render_newton.cpp
#include "render_newton.h"
#include <vector>
#include <stdexcept>
#include "newtonApprox.h"   // your ISPC header

struct Points {
    std::vector<double> re, im;
    std::vector<short>  approaches;
    std::vector<float>  convSpeed;
    void resize(size_t n) { re.resize(n); im.resize(n); approaches.resize(n); convSpeed.resize(n); }
};

struct Roots {
    std::vector<double> reRoots, imRoots;
    void resize(short p) { reRoots.resize(p); imRoots.resize(p); }
};

bool render_newton_frame(
    int power,
    size_t width,
    size_t height,
    unsigned short max_iter,
    double min_step2,
    FrameBuff& out_fb,
    std::string* err_msg
) {
    try {
        out_fb.resize(width, height);
        Points pts; pts.resize(width*height);
        Roots roots; roots.resize(static_cast<short>(power));

        ispc::approxISPC(width, height,
                         roots.reRoots.data(), roots.imRoots.data(),
                         static_cast<unsigned short>(power),
                         pts.re.data(), pts.im.data(),
                         out_fb.red.data(), out_fb.green.data(), out_fb.blue.data(),
                         max_iter, min_step2);
        return true;
    } catch (const std::exception& e) {
        if (err_msg) *err_msg = e.what();
        return false;
    }
}
