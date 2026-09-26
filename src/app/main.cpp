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
        
        EzMessage ez_message;

        constexpr auto data = std::to_array<std::uint8_t>({
            0xAA,   // BOF
            0x01,   // DEV_ID
            0x30, // CMD_NUMBER
        });
        
        
        std::copy(data.begin(),
          data.end(),
          ez_message.payload_.begin());

        ez_message.payload_len_ = data.size();
        
        std::jthread poll_thread{
            [&ez_protocol, &ez_message]
            {
                while (true)
                {
                    ez_protocol.send(ez_message);

                    std::this_thread::sleep_for(
                        std::chrono::seconds{1}
                    );
                }
            }
        };

        // run process forever
        while (true)
        {
            EzMessage message = ez_protocol.receive();   // blocking

            ez_message_processor(message);
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

static void ez_message_processor(const EzMessage &message)
{
    const std::int32_t temperature_centi_deg{
        static_cast<std::int32_t>(
            (static_cast<std::uint32_t>(message.payload_[0]) << 24U) |
            (static_cast<std::uint32_t>(message.payload_[1]) << 16U) |
            (static_cast<std::uint32_t>(message.payload_[2]) << 8U)  |
             static_cast<std::uint32_t>(message.payload_[3])
        )
    };

    const std::uint32_t pressure_pa{
        (static_cast<std::uint32_t>(message.payload_[4]) << 24U) |
        (static_cast<std::uint32_t>(message.payload_[5]) << 16U) |
        (static_cast<std::uint32_t>(message.payload_[6]) << 8U)  |
         static_cast<std::uint32_t>(message.payload_[7])
    };

    const std::uint32_t humidity_milli_pct{
        (static_cast<std::uint32_t>(message.payload_[8])  << 24U) |
        (static_cast<std::uint32_t>(message.payload_[9])  << 16U) |
        (static_cast<std::uint32_t>(message.payload_[10]) << 8U)  |
         static_cast<std::uint32_t>(message.payload_[11])
    };

    std::cerr << "Temperature: "
              << temperature_centi_deg / 100.0
              << " C | Pressure: "
              << pressure_pa / 100.0
              << " hPa | Humidity: "
              << humidity_milli_pct / 1000.0
              << " %\n";
}