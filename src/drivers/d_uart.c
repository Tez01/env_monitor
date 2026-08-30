/******************************************************************************
* Filename              :   d_uart.c
* Author                :   Tej
* Origin Date           :   18-08-26
* Version               :   1.0.0
*
* Description:
*   This file contains the Linux UART driver interface used for serial
*   communication with the STM32.
*******************************************************************************/

/******************************************************************************
* Includes
*******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include "fcntl.h"
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
/******************************************************************************
* Application Constants
*******************************************************************************/


/******************************************************************************
* Macros
*******************************************************************************/
#define BME_RESPONSE_SIZE    12U

/******************************************************************************
* Typedefs
*******************************************************************************/

/******************************************************************************
* Static Constants
*******************************************************************************/
static const char uart_device_path_pi[] =
    "/dev/ttyACM0";

static const char uart_device_path_pc[] =
    "/dev/serial/by-id/"
    "usb-STMicroelectronics_STM32_STLink_0671FF485157808667075619-if02";
  
const char *uart_device_path = NULL;static 
/******************************************************************************
* Static Global Variables
*******************************************************************************/
static struct termios tty_settings;


/******************************************************************************
* Exposed Variables
*******************************************************************************/


/******************************************************************************
* Static Function Prototypes
*******************************************************************************/
static void print_error(char *error_msg, int error_number);
static void print_debug(char *debug_msg);

static void print_bme_data(int32_t temperature_centi_deg,
                           uint32_t pressure_pa,
                           uint32_t humidity_milli_pct);

/******************************************************************************
* Function Definitions
*******************************************************************************/



int main(int argc, char *argv[]){
    // Select device node based on platform
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <pi|pc>\n", argv[0]);
        goto ERROR;
    }

    if (strcmp(argv[1], "pi") == 0)
    {
        uart_device_path = uart_device_path_pi;
    }
    else if (strcmp(argv[1], "pc") == 0)
    {
        uart_device_path = uart_device_path_pc;
    }
    else
    {
        fprintf(stderr, "Invalid platform: %s\n", argv[1]);
        fprintf(stderr, "Usage: %s <pi|pc>\n", argv[0]);
        goto ERROR;
    }

    char fuser_cmd[512];

    snprintf(fuser_cmd,
            sizeof(fuser_cmd),
            "fuser -s %s",
            uart_device_path);
            
    // Check if uart owned by some process
    int status = system(fuser_cmd);
    if (status == -1)
    {
        /* system() itself failed */
        print_error("fuser command failed", errno);
        goto ERROR;
    }
    else if (WIFEXITED(status) != 0)
    {
        int exit_code = WEXITSTATUS(status);

        if (exit_code == 0)
        {
            /* UART is in use */
            print_debug("UART is in use by some other process. Please close the process and try again.");
            goto ERROR;
        }
        else if (exit_code == 1)
        {
            /*
            * UART is free.
            * Continue with open().
            */
            print_debug("UART is free");
        }
        else
        {
            /*
            * Unexpected fuser result.
            */
            print_error("Unexpected fuser result", errno);
            goto ERROR;
        }
    }
    else
    {
        /* command was terminated abnormally */
        print_error("fuser command failed", errno);
        goto ERROR;
    }

    // Open uart device
    int fd = open(uart_device_path,
                    O_RDWR | O_NOCTTY);
    if(fd < 0){
        // error
        print_error("Device open failed", errno);
        goto ERROR;
    }



    // Exclusively own the device
    if (ioctl(fd, TIOCEXCL) == -1)
    {
        print_error("Failed to exclusively own device", errno);
        goto ERROR;
    }

    print_debug("File opened");
    
    // Configure uart settings
    if(tcgetattr(fd, &tty_settings) != 0){
        print_error("Failed to get tty attributes", errno);
        goto ERROR;
    }


    cfmakeraw(&tty_settings);//  give raw bytes instead of behaving as human terminal

    cfsetispeed(&tty_settings, B115200);

    cfsetospeed(&tty_settings, B115200);

    tty_settings.c_cflag &= (tcflag_t)~CSIZE;

    tty_settings.c_cflag |= CS8;
    tty_settings.c_cflag &= (tcflag_t)~PARENB;
    tty_settings.c_cflag &= (tcflag_t)~CSTOPB;
    tty_settings.c_cflag &= (tcflag_t)~CRTSCTS;
    tty_settings.c_cflag |= CREAD | CLOCAL;
    tty_settings.c_iflag &= (tcflag_t)(~(IXON | IXOFF | IXANY));
    tty_settings.c_cc[VMIN] = 1;    // block until atleast 1 byte is received
    tty_settings.c_cc[VTIME] = 0; // No timeout
    
    if(tcsetattr(fd, TCSANOW, &tty_settings) != 0){
        print_error("Failed to set tty attributes", errno);
        goto ERROR;
    }   


    // Flush old data
    if(tcflush(fd, TCIFLUSH) != 0){
        print_error("Failed to flush tty", errno);
        goto ERROR;
    }


    
    while(1){

        // Send GET_BME_DATA cmd
        uint8_t tx_buffer[] = 
        {
            0xAA,   // BOF
            0x01,   // DEV_ID
            0x30, // CMD_NUMBER
        };
        
        size_t total_written = 0;
        
        while(total_written < sizeof(tx_buffer)){
            ssize_t bytes_written = write(fd, tx_buffer + total_written, sizeof(tx_buffer) - total_written);
            if(bytes_written < 0){
                print_error("Failed to write to tty", errno);
                goto ERROR;
            }
            else if(bytes_written == 0){
                print_debug("No data written");
                continue;
            }
            total_written += (size_t)bytes_written;
        }
        
        print_debug("Data sent");
        
        uint8_t rx_buffer[20];
        
        size_t total_read = 0U;

        while (total_read < BME_RESPONSE_SIZE)
        {
            ssize_t bytes_read = read(fd,
                                    rx_buffer + total_read,
                                    BME_RESPONSE_SIZE - total_read);

            if (bytes_read < 0)
            {
                print_error("Failed to read from tty", errno);
                goto ERROR;
            }
            else if (bytes_read == 0)
            {
                print_debug("No data received");
                continue;
            }

            total_read += (size_t)bytes_read;
        }

        print_debug("Data received");

        printf("RX Raw: ");

        for (size_t i = 0U; i < total_read; i++)
        {
            printf("%02X ", rx_buffer[i]);
        }

        printf("\n");

        int32_t temperature_centi_deg =
            (int32_t)(
                ((uint32_t)rx_buffer[0] << 24U) |
                ((uint32_t)rx_buffer[1] << 16U) |
                ((uint32_t)rx_buffer[2] << 8U)  |
                ((uint32_t)rx_buffer[3]));

        uint32_t pressure_pa =
            ((uint32_t)rx_buffer[4] << 24U) |
            ((uint32_t)rx_buffer[5] << 16U) |
            ((uint32_t)rx_buffer[6] << 8U)  |
            ((uint32_t)rx_buffer[7]);

        uint32_t humidity_milli_pct =
            ((uint32_t)rx_buffer[8] << 24U)  |
            ((uint32_t)rx_buffer[9] << 16U)  |
            ((uint32_t)rx_buffer[10] << 8U)  |
            ((uint32_t)rx_buffer[11]);

        print_bme_data(temperature_centi_deg, pressure_pa, humidity_milli_pct);

        sleep(1);
    }

ERROR:
    return -1;

}


static void print_error(char *error_msg, int error_number){

    // syslog(LOG_ERR, error_msg, strerror(error_number));
    fprintf(stderr, "ERROR: %s: %s\n", error_msg, strerror(error_number));

}


static void print_debug(char *debug_msg){

    // syslog(LOG_ERR, error_msg, strerror(error_number));
    fprintf(stderr, "DEBUG: %s\n", debug_msg);

}


static void print_bme_data(int32_t temperature_centi_deg,
                           uint32_t pressure_pa,
                           uint32_t humidity_milli_pct)
{
    printf("\n");
    printf("BME280 Data\n");
    printf("-----------------------------\n");

    printf("Temperature : %.2f C\n",
           (double)temperature_centi_deg / 100.0);

    printf("Pressure    : %u Pa\n",
           pressure_pa);

    printf("Humidity    : %.3f %%\n",
           (double)humidity_milli_pct / 1000.0);

    printf("-----------------------------\n");
}
