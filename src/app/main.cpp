#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <chrono>
#include <thread>
#include <cstring>

#include <termios.h>

#include "f_transport.hpp"
#include "f_serial_port.hpp"
#include "f_ez.hpp"

#include "u_syslog.hpp"

static constexpr std::string_view uart_device_path_pi{
    "/dev/ttyACM0"
};

static constexpr std::string_view uart_device_path_pc{
    "/dev/serial/by-id/"
    "usb-STMicroelectronics_STM32_STLink_0671FF485157808667075619-if02"
};

static void ez_message_processor(const EzMessage &message);

static void print_usage(const char* program_name)
{
    std::cerr
        << "Usage: "
        << program_name
        << " --serial <pi|pc|device-path>\n";
}

int main(int argc, char* argv[])
{
    SysLogger sys_logger{};

    if (argc != 3)
    {
        print_usage(argv[0]);
        sys_logger.error("Invalid command-line arguments");

        return EXIT_FAILURE;
    }

    const std::string_view transport_option{argv[1]};
    const std::string_view transport_value{argv[2]};

    try
    {
        std::unique_ptr<Transport> transport{};

        /*
         * Select and initialize transport.
         */
        if (transport_option == "--serial")
        {
            std::string_view device_path{};

            if (transport_value == "pi")
            {
                device_path = uart_device_path_pi;
            }
            else if (transport_value == "pc")
            {
                device_path = uart_device_path_pc;
            }
            else
            {
                device_path = transport_value;
            }

            transport = std::make_unique<SerialPort>(
                B115200,
                device_path,
                sys_logger
            );

            sys_logger.debug("SerialPort created successfully");

            // Create Ez Protocol thread




        }
        else
        {
            print_usage(argv[0]);
            sys_logger.error("Unknown transport option");

            return EXIT_FAILURE;
        }


        EzProtocol ez_protocol{sys_logger,
                            *transport,
                            ez_message_processor};

        
        EzMessage ez_message{
            .command_num_ = 1,
            .payload_ = {},
            .payload_len_ = 0
        };


        ez_protocol.send(ez_message);

        // PeriodicPoll periodic_poll{
        //     std::chrono::milliseconds{500},
        //     ez_protocol
        // };

        // run process forever
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds{1});
        }

    }
    catch (const std::exception& e)
    {
        std::string message{"Exception occurred: "};
        message += e.what();

        sys_logger.error(message.c_str());

        return EXIT_FAILURE;
    }



    return EXIT_SUCCESS;
}

static void ez_message_processor(const EzMessage &message){
    std::cerr   << "Message processed: " 
                << message.command_num_ 
                << '\n';
}