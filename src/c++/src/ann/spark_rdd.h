// Parse output of Spark ALS RDDs that have the following format:
//
// (1728,List(-0.031882576644420624, 0.09457508474588394, ...))
//

#include <boost/config/warning_disable.hpp>
#include <boost/filesystem.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "ann/space.h"

namespace spark
{
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
namespace fs = boost::filesystem;
//namespace fs = std::filesystem;

using qi::float_;
using qi::long_;
using qi::phrase_parse;
using qi::_1;
using ascii::space;
using phoenix::push_back;

template <typename Iterator>
bool parse_List(Iterator first, Iterator last, unsigned int& rid, std::vector<float>& v)
{
    bool r = phrase_parse(first, last,
        //  Begin grammar
        (
            "(" >> long_ >> ',' >> "List(" >>
            float_[push_back(phoenix::ref(v), _1)]
                >> *(',' >> float_[push_back(phoenix::ref(v), _1)])
                >> "))"
        )
        ,
        //  End grammar
        space, rid);

    if (first != last) {    // fail if we did not get a full match
        return false;
    }
    return r;
}

template <typename Iterator>
bool parse_WrappedArray(Iterator first, Iterator last, unsigned int& rid, std::vector<float>& v)
{
    bool r = phrase_parse(first, last,
        //  Begin grammar
        (
            long_ >> ',' >> "\"WrappedArray(" >>
            float_[push_back(phoenix::ref(v), _1)]
                >> *(',' >> float_[push_back(phoenix::ref(v), _1)])
                >> ")\""
        )
        ,
        //  End grammar
        space, rid);

    if (first != last) {    // fail if we did not get a full match
        return false;
    }
    return r;
}

bool parse_SparkLine(const std::string& line, unsigned int& rid, std::vector<float>& v);
void LoadFile(const fs::path& filepath, Space<AnyID>* space);
void LoadFiles(const char* path, Space<AnyID>* space);
}
