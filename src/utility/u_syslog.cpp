#include "u_syslog.hpp"

#include <iostream>

#include <syslog.h>


SysLogger::SysLogger(){
    ::openlog("mold_detector_log", LOG_PID , LOG_USER);
    
}

SysLogger::~SysLogger(){
    ::closelog();
}


void SysLogger::error(std::string_view message) const{
    ::syslog(LOG_ERR, "%.*s", static_cast<int>(message.size()), message.data());
    std::cerr << "ERROR: " << message << '\n';
}


void SysLogger::debug(std::string_view message) const{
    ::syslog(LOG_DEBUG, "%.*s", static_cast<int>(message.size()), message.data());
    std::cerr << "DEBUG: " << message << '\n';
}

