/******************************************************************************
* Filename              :   f_bme_cpp.cpp
* Author                :   Tej
* Origin Date           :   12-09-26
* Version               :   1.0.0
*
* Description:
*   This file contains functions for processing BME280 data
*******************************************************************************/

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "f_bme_cpp.h"

#include <cstdint>

#include <iostream>

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

class BmeData
{
public:
    BmeData(std::int32_t temperature,
            std::uint32_t pressure,
            std::uint32_t humidity):
        
        temperature_centi_deg{temperature},
        pressure_pa{pressure},
        humidity_milli_pct{humidity}
    {

    }
        
    std::int32_t temperature_centi_deg{};
    std::uint32_t pressure_pa{};
    std::uint32_t humidity_milli_pct{};
};


void f_bme_cpp_process_data(std::int32_t temperature_centi_deg,
                    std::uint32_t pressure_pa,
                    std::uint32_t humidity_milli_pct){

    BmeData bme_data{temperature_centi_deg,
                        pressure_pa,
                        humidity_milli_pct};

    std::cout
        << "temp(C): " << bme_data.temperature_centi_deg << '\n'
        << "pressure(Pa): " << bme_data.pressure_pa << '\n'
        << "humidity(m%): " << bme_data.humidity_milli_pct << '\n';
}