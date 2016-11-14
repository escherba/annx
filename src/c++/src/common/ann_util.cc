#include <iostream>
#include <string>

#include "common/ann_util.h"

namespace annx{
namespace util {
void printProgBar(size_t idx, size_t total) {

    auto percent = (idx * 100) / total;
    std::string bar;

    for(size_t i = 0; i < 50; i++) {
        if( i < (percent/2)){
            bar.replace(i,1,"=");
        } else if( i == (percent/2)){
            bar.replace(i,1,">");
        } else{
            bar.replace(i,1," ");
        }
    }

    std::cerr<< "\r" "[" << bar << "] ";
    std::cerr.width(3);
    std::cerr<< percent << "%     " << std::flush;
}
}
}
