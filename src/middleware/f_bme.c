/******************************************************************************
* Filename              :   f_bme.c
* Author                :   Tej
* Origin Date           :   30-08-26
* Version               :   1.0.0
*
* Description:
*   This file contains functions for processing and sending BME280 data
*   received by the Linux application.
*******************************************************************************/

/******************************************************************************
* Includes
*******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <curl/curl.h>

#include "f_bme.h"

/******************************************************************************
* Application Constants
*******************************************************************************/


/******************************************************************************
* Macros
*******************************************************************************/


/******************************************************************************
* Typedefs
*******************************************************************************/


/******************************************************************************
* Static Global Variables
*******************************************************************************/


/******************************************************************************
* Exposed Variables
*******************************************************************************/


/******************************************************************************
* Static Function Prototypes
*******************************************************************************/


/******************************************************************************
* Function Definitions
*******************************************************************************/
void f_bme__print_data(int32_t temperature_centi_deg,
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


int f_bme__send_data(int32_t temperature_centi_deg,
                     uint32_t pressure_pa,
                     uint32_t humidity_milli_pct)
{
    CURL *curl_handle = NULL;

    (void)temperature_centi_deg;
    (void)pressure_pa;
    (void)humidity_milli_pct;

    curl_handle = curl_easy_init();

    if (curl_handle == NULL)
    {
        fprintf(stderr, "ERROR: curl_easy_init failed\n");
        return -1;
    }

    fprintf(stderr, "DEBUG: libcurl initialized successfully\n");

    curl_easy_cleanup(curl_handle);

    return 0;
}
