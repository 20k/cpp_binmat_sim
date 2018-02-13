#ifndef UI_UTIL_HPP_INCLUDED
#define UI_UTIL_HPP_INCLUDED

#include <sstream>
#include <stdio.h>
#include <ctype.h>

#if 0
namespace ImGui
{
    inline
    void TextColored(const std::string& str, vec3f col)
    {
        TextColored(ImVec4(col.x(), col.y(), col.z(), 1), str.c_str());
    }
}
#endif

inline bool file_exists (const std::string& name)
{
    std::ifstream f(name.c_str());
    return f.good();
}

inline
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

inline
std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

inline
std::string strip_whitespace(std::string in)
{
    if(in.size() == 0)
        return in;

    while(in.size() > 0 && isspace(in[0]))
    {
        in.erase(in.begin());
    }

    while(in.size() > 0 && isspace(in.back()))
    {
        in.pop_back();
    }

    return in;
}

#endif // UI_UTIL_HPP_INCLUDED
