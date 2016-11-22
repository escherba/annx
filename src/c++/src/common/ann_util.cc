#include <cstdlib>
#include <iostream>
#include <string>

#include "common/ann_util.h"

namespace ann {
namespace util {

ProgBar::ProgBar(size_t total)
    : pv_(1), total_(total), percent_(0)
{
    std::system("setterm -cursor off");
}

ProgBar::~ProgBar() {
    std::cerr << std::endl;
    std::system("setterm -cursor on");
}

void ProgBar::update(size_t count) {

    // with changes after
    // https://nakkaya.com/2009/11/08/command-line-progress-bar/
    //
    auto percent = (pv_ * 100) / total_;
    pv_ += count;

    if (percent == percent_) {
        return;
    }

    percent_ = percent;
    std::string bar;

    for (size_t i = 0; i < 50; i++) {
        if (i < (percent / 2)) {
            bar.replace(i, 1, "=");
        } else if (i == (percent / 2)) {
            bar.replace(i, 1, ">");
        } else {
            bar.replace(i, 1, " ");
        }
    }

    std::cerr << "\r" "[" << bar << "] ";
    std::cerr.width(3);
    std::cerr << percent << "%     " << std::flush;
}
}
}
