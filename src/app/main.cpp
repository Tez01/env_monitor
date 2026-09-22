#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <chrono>
#include <thread>
#include <cstring>

#include <termios.h>

#include "u_syslog.hpp"
#include "f_transport.hpp"
#include "f_serial_port.hpp"

static constexpr std::string_view uart_device_path_pi{
    "/dev/ttyACM0"
};

static constexpr std::string_view uart_device_path_pc{
    "/dev/serial/by-id/"
    "usb-STMicroelectronics_STM32_STLink_0671FF485157808667075619-if02"
};

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


            while (true)
            {
                // Send GET_BME_DATA cmd
                auto tx_buffer = std::to_array<std::uint8_t>({
                    0xAA,   // BOF
                    0x01,   // DEV_ID
                    0x30, // CMD_NUMBER
                });

                const std::size_t bytes_written{
                    transport->write(tx_buffer, 3)
                };
                std::cout << "Transmitted bytes: " << bytes_written << '\n';

                std::array<std::uint8_t, 20> rx_buffer{};
                const std::size_t bytes_read{
                    transport->read(rx_buffer, 12)
                };

                if (bytes_read == 12)
                {

                    const std::int32_t temperature_centi_deg{
                        static_cast<std::int32_t>(
                            (static_cast<std::uint32_t>(rx_buffer[0]) << 24U) |
                            (static_cast<std::uint32_t>(rx_buffer[1]) << 16U) |
                            (static_cast<std::uint32_t>(rx_buffer[2]) << 8U)  |
                            static_cast<std::uint32_t>(rx_buffer[3]))
                    };

                    const std::uint32_t pressure_pa{
                        (static_cast<std::uint32_t>(rx_buffer[4]) << 24U) |
                        (static_cast<std::uint32_t>(rx_buffer[5]) << 16U) |
                        (static_cast<std::uint32_t>(rx_buffer[6]) << 8U)  |
                        static_cast<std::uint32_t>(rx_buffer[7])
                    };

                    const std::uint32_t humidity_milli_pct{
                        (static_cast<std::uint32_t>(rx_buffer[8]) << 24U) |
                        (static_cast<std::uint32_t>(rx_buffer[9]) << 16U) |
                        (static_cast<std::uint32_t>(rx_buffer[10]) << 8U) |
                        static_cast<std::uint32_t>(rx_buffer[11])
                    };

                    const double temperature_c{
                        static_cast<double>(temperature_centi_deg) / 100.0
                    };

                    const double pressure_hpa{
                        static_cast<double>(pressure_pa) / 100.0
                    };

                    const double humidity_pct{
                        static_cast<double>(humidity_milli_pct) / 1000.0
                    };


                    std::cout << std::fixed << std::setprecision(2)
                            << "Temperature: " << temperature_c << " °C"
                            << " | Humidity: " << humidity_pct << " %"
                            << " | Pressure: " << pressure_hpa << " hPa"
                            << "\n\n" << std::flush;
                }

                std::this_thread::sleep_for(std::chrono::seconds{1});
            }

        }
        else
        {
            print_usage(argv[0]);
            sys_logger.error("Unknown transport option");

            return EXIT_FAILURE;
        }

        // /*
        //  * Everything below this point is transport independent.
        //  */
        // EzProtocol ez_protocol{*transport};

        // PeriodicPoll periodic_poll{
        //     std::chrono::milliseconds{500},
        //     ez_protocol
        // };

        // // run process forever
        // while (true)
        // {
        //     std::this_thread::sleep_for(std::chrono::seconds{1});
        // }

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