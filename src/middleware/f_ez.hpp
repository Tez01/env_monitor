#include <thread>
#include <array>
#include <cstdint>
#include <cstddef>

#include "u_syslog.hpp"
#include "u_blocking_queue.hpp"

#include "a_constants.h"

struct EzMessage{
    std::uint16_t command_num_;
    std::array<std::uint8_t, EZ_MAX_PAYLOAD_LEN> payload_;
    std::size_t payload_len_;
};

class EzProtocol{

    public:
        EzProtocol(const SysLogger &sys_logger,
                    Transport &transport,
                    void (*message_processor)(const EzMessage&)):
            sys_logger_{sys_logger},
            transport_{transport},
            message_processor_{message_processor},
            rx_thread_(&EzProtocol::rx_worker, this),
            tx_thread_(&EzProtocol::tx_worker, this)
        {

        }

        void message_sender_(const EzMessage &message){
            std::string log_message = "Message sent: " +
                std::to_string(message.command_num_);
            sys_logger_.debug(log_message);

            log_message = "Adding message to rx queue: " +
                std::to_string(message.command_num_);
            sys_logger_.debug(log_message);

            rx_queue_.add(message);
        }

        void send(const EzMessage &message){
            std::string log_message = "Message about to be sent: " +
                std::to_string(message.command_num_);

            sys_logger_.debug(log_message);

            tx_queue_.add(message);
        }
    private:   
        const SysLogger &sys_logger_;
        Transport &transport_;
        BlockingQueue<EzMessage> tx_queue_;
        BlockingQueue<EzMessage> rx_queue_;
        void (*message_processor_)(const EzMessage&);

        std::jthread rx_thread_;

        void rx_worker(){
            while(true){
                // wait on rx queue
                EzMessage message = rx_queue_.get(); // This will sleep this thread if no message

                // something in queue

                std::string log_message = "Got message from rx queue: " +
                    std::to_string(message.command_num_);
                sys_logger_.debug(log_message);

                // process it
                message_processor_(message);

            }
        }

        std::jthread tx_thread_;
        void tx_worker(){
            while(true){
                // wait on tx queue
                EzMessage message = tx_queue_.get(); // This will sleep this thread if no message
                // something in queue
                // send it
                std::string log_message = "TX worker got message from queue: " +
                std::to_string(message.command_num_);
                
                sys_logger_.debug(log_message);

                message_sender_(message);
            }
        }

};