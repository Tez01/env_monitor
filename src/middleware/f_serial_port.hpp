#pragma once

#include <sys/ioctl.h>
#include <fcntl.h>

#include "f_transport.hpp"

#include "u_syslog.hpp"



class FileDescriptor{
    public:
        explicit FileDescriptor(int fd):
            fd_{fd}{
            
        }

        FileDescriptor(const FileDescriptor&) = delete;
        FileDescriptor& operator=(const FileDescriptor&) = delete;

        ~FileDescriptor() noexcept{
            if(fd_ >= 0){
                ::close(fd_);
            }
        }

        int get() const{
            return fd_;
        }

    private:
        int fd_;

};


class SerialPort: public Transport{
    public:
        SerialPort(speed_t baud, 
                    std::string_view device_path,
                    const SysLogger &sys_logger):
                    Transport{},
                    device_path_{device_path},
                    baud_{baud},
                    fd_{open_device(device_path_)}{

            // Exclusively own the device
            if (::ioctl(fd_.get(), TIOCEXCL) == -1)
            {
                throw std::system_error{
                    errno,
                    std::generic_category(),
                    "Failed to exclusively own device"
                };
            }

            sys_logger.debug("File opened");

            // Configure uart settings
            struct termios tty_settings{};
            if(::tcgetattr(fd_.get(), &tty_settings) != 0){
                throw std::system_error{
                    errno,
                    std::generic_category(),
                    "Failed to get tty attributes"
                };
            }

            ::cfmakeraw(&tty_settings);//  give raw bytes instead of behaving as human terminal

            if (::cfsetispeed(&tty_settings, baud_) == -1)
            {
                throw std::system_error{
                    errno,
                    std::generic_category(),
                    "Failed to set input baud rate"
                };
            }

            if (::cfsetospeed(&tty_settings, baud_) == -1)
            {
                throw std::system_error{
                    errno,
                    std::generic_category(),
                    "Failed to set output baud rate"
                };
            }

            tty_settings.c_cflag &= (tcflag_t)~CSIZE;

            tty_settings.c_cflag |= CS8;
            tty_settings.c_cflag &= (tcflag_t)~PARENB;
            tty_settings.c_cflag &= (tcflag_t)~CSTOPB;
            tty_settings.c_cflag &= (tcflag_t)~CRTSCTS;
            tty_settings.c_cflag |= CREAD | CLOCAL;
            tty_settings.c_iflag &= (tcflag_t)(~(IXON | IXOFF | IXANY));
            tty_settings.c_cc[VMIN] = 1;    // block until atleast 1 byte is received
            tty_settings.c_cc[VTIME] = 0; // No timeout

                
            if(::tcsetattr(fd_.get(), TCSANOW, &tty_settings) != 0){
                throw std::system_error{
                    errno,
                    std::generic_category(),
                    "Failed to set tty attributes",
                };
            }  

            // Flush old data
            if(::tcflush(fd_.get(), TCIFLUSH) != 0){
                throw std::system_error{
                    errno,
                    std::generic_category(),
                    "Failed to flush tty",
                };
            }

        }

        std::size_t read(std::span<std::uint8_t> buffer, std::size_t bytes_to_read) override{
            if(bytes_to_read > buffer.size()){
                throw std::invalid_argument{
                    "bytes_to_read exceeds buffer size"
                };
            }
            std::size_t total_read{0};

            while (total_read < bytes_to_read)
            {
                const ssize_t bytes_read{::read(fd_.get(),
                                        buffer.data() + total_read,
                                        bytes_to_read - total_read)};
                
                if (bytes_read < 0)
                {
                    if(errno == EINTR){
                        continue;
                    }

                    throw std::system_error{
                        errno,
                        std::generic_category(),
                        "Failed to read from tty"
                    };
                }

                if (bytes_read == 0)
                {
                    // No data received
                    break;
                }

                total_read += static_cast<std::size_t>(bytes_read);
            }

            return total_read;
        }

        
        std::size_t write(std::span<const std::uint8_t> buffer, std::size_t bytes_to_write) override{
            if(bytes_to_write > buffer.size()){
                throw std::invalid_argument{
                    "bytes to write exceeds buffer size"
                };
            }
            std::size_t total_num_bytes_written{0};

            while(bytes_to_write > 0){
                const ssize_t num_bytes_written{
                    ::write(fd_.get(), 
                    buffer.data() + total_num_bytes_written,
                    bytes_to_write)
                };

                if(num_bytes_written < 0){
                    if (errno == EINTR)
                    {
                        continue;
                    }
                    
                    throw std::system_error{
                        errno,
                        std::generic_category(),
                        "Failed to write bytes"
                    };
                }

                if(num_bytes_written == 0){
                    break;
                }

                const auto written{
                    static_cast<std::size_t>(num_bytes_written)
                };

                total_num_bytes_written += written;
                bytes_to_write -= written;
            }

            return total_num_bytes_written;
        }

    private:

        static int open_device(const std::string &device_path){
            const int fd{::open(
                device_path.c_str(),
                O_RDWR | O_NOCTTY)};
            
            if(fd < 0){
                throw std::system_error{
                    errno,
                    std::generic_category(),
                    "Device open failed"
                };
            }
            return fd;
        }

        std::string device_path_;    
        speed_t baud_;
        FileDescriptor fd_;
};

