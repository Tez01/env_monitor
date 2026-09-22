#pragma once

#include <string_view>

class SysLogger{
    public:
        SysLogger();

        ~SysLogger();

        void error(std::string_view message) const;

        void debug(std::string_view message) const;
};
