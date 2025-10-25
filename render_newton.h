// render_newton.h
#pragma once
#include <cstddef>
#include <string>
#include <vector>


struct FrameBuff {
    size_t width{}, height{};
    std::vector<unsigned char> red, green, blue;
    FrameBuff() = default;
    FrameBuff(size_t w, size_t h) { resize(w, h); }
    void resize(size_t w, size_t h) {
        width = w; height = h;
        red.resize(w*h); green.resize(w*h); blue.resize(w*h);
    }
};

bool render_newton_frame(
    int power,
    size_t width,
    size_t height,
    unsigned short max_iter,
    double min_step2,
    FrameBuff& out_fb,      // filled on success
    std::string* err_msg    // optional
);
