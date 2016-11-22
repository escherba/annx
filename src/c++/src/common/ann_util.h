#pragma once

#include <cstddef>

namespace ann {
namespace util {

class ProgBar {
    public:
    ProgBar(size_t total);
    ~ProgBar();

    void update(size_t count=1);

    private:
    size_t pv_;
    size_t total_;
    size_t percent_;
};

}
}
