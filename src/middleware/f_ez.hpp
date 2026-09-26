#include <thread>
#include <array>
#include <cstdint>
#include <cstddef>

#include "u_syslog.hpp"
#include "u_blocking_queue.hpp"

#include "a_constants.h"

enum EzMessageField{
    BOF = 0,
    DEVICE_ID,
    CMD_TYPE,
    DATA_BYTE_1
};

struct EzMessage{
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


        void send(const EzMessage &message){
            std::string log_message = "Message about to be sent: " +
                std::to_string(message.payload_[2]);

            sys_logger_.debug(log_message);

            tx_queue_.add(message);
        }

        EzMessage receive(){

            return rx_queue_.get();
        }

    private:   
        const SysLogger &sys_logger_;
        Transport &transport_;
        BlockingQueue<EzMessage> tx_queue_;
        BlockingQueue<EzMessage> rx_queue_;
        void (*message_processor_)(const EzMessage&);

        std::jthread rx_thread_;

        void rx_worker()
        {
            constexpr std::size_t RESPONSE_SIZE{12};

            std::array<std::uint8_t, RESPONSE_SIZE> buffer{};
            std::size_t total_received{0};

            while (true)
            {
                const std::size_t bytes_received{
                    transport_.read(
                        std::span<std::uint8_t>{
                            buffer.data() + total_received,
                            buffer.size() - total_received
                        }
                    )
                };

                total_received += bytes_received;

                if (total_received == RESPONSE_SIZE)
                {
                    // We now have ONE complete 12-byte EZ response.

                    EzMessage message{};
                    std::copy(buffer.begin(),
                            buffer.end(),
                            message.payload_.begin());

                    message.payload_len_ = RESPONSE_SIZE;

                    rx_queue_.add(message);

                    total_received = 0;
                }
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
                std::to_string(message.payload_[CMD_TYPE]);
                
                sys_logger_.debug(log_message);

                transport_.write(message.payload_, message.payload_len_);

                log_message = "Message sent: " +
                std::to_string(message.payload_[CMD_TYPE]);
                
                sys_logger_.debug(log_message);
            }
        }

};