/**
 * Copyright (c) 2016 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 * @defgroup tw_scanner main.c
 * @{
 * @ingroup nrf_twi_example
 * @brief TWI Sensor Example main file.
 *
 * This file contains the source code for a sample application using TWI.
 *
 */

#include <stdio.h> // printf
#include "boards.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "sgp30.h"
#include <unistd.h> // sleep

unsigned int sleep(unsigned int seconds)
{
    nrf_delay_ms(seconds * 1000);
    return seconds;
}

/**
 * @brief Function for main application entry.
 */
int main(void)
{

    u16 i = 0;
    s16 err;
    u16 tvoc_ppb, co2_eq_ppm;
    u32 iaq_baseline;
    u16 ethanol_raw_signal, h2_raw_signal;

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    NRF_LOG_INFO("TWI SPG30 started.");
    NRF_LOG_FLUSH();

    const char *driver_version = sgp30_get_driver_version();
    if (driver_version)
    {
        NRF_LOG_INFO("SGP30 driver version %s\n", driver_version);
    }
    else
    {
        NRF_LOG_INFO("fatal: Getting driver version failed\n");
        return -1;
    }
    NRF_LOG_FLUSH();
    /* Busy loop for initialization. The main loop does not work without
     * a sensor. */
    while (sgp30_probe() != STATUS_OK)
    {
        NRF_LOG_INFO("SGP sensor probing failed\n");
        sleep(1);
    }

    NRF_LOG_INFO("SGP sensor probing successful\n");
    NRF_LOG_FLUSH();

    u16 feature_set_version;
    u8 product_type;
    err = sgp30_get_feature_set_version(&feature_set_version, &product_type);
    if (err == STATUS_OK)
    {
        NRF_LOG_INFO("Feature set version: %u\n", feature_set_version);
        NRF_LOG_INFO("Product type: %u\n", product_type);
    }
    else
    {
        NRF_LOG_INFO("sgp30_get_feature_set_version failed!\n");
    }
    NRF_LOG_FLUSH();
    u64 serial_id;
    err = sgp30_get_serial_id(&serial_id);
    if (err == STATUS_OK)
    {
        NRF_LOG_INFO("SerialID: %lld", serial_id);
    }
    else
    {
        NRF_LOG_INFO("sgp30_get_serial_id failed!\n");
    }
    NRF_LOG_FLUSH();
    /* Read gas raw signals */
    err = sgp30_measure_raw_blocking_read(&ethanol_raw_signal,
                                          &h2_raw_signal);
    if (err == STATUS_OK)
    {
        /* Print ethanol raw signal and h2 raw signal */
        NRF_LOG_INFO("Ethanol raw signal: %u\n", ethanol_raw_signal);
        NRF_LOG_INFO("H2 raw signal: %u\n", h2_raw_signal);
    }
    else
    {
        NRF_LOG_INFO("error reading raw signals\n");
    }
    NRF_LOG_FLUSH();
    /* Consider the two cases (A) and (B):
     * (A) If no baseline is available or the most recent baseline is more than
     *     one week old, it must discarded. A new baseline is found with
     *     sgp30_iaq_init() */
    err = sgp30_iaq_init();
    if (err == STATUS_OK)
    {
        NRF_LOG_INFO("sgp30_iaq_init done\n");
    }
    else
    {
        NRF_LOG_INFO("sgp30_iaq_init failed!\n");
    }
    NRF_LOG_FLUSH();
    /* (B) If a recent baseline is available, set it after sgp30_iaq_init() for
     * faster start-up */
    /* IMPLEMENT: retrieve iaq_baseline from presistent storage;
     * err = sgp30_set_iaq_baseline(iaq_baseline);
     */

    while (true)
    { /*
        * IMPLEMENT: get absolute humidity to enable humidity compensation
        * u32 ah = get_absolute_humidity(); // absolute humidity in mg/m^3
        * sgp30_set_absolute_humidity(ah);
        */

        err = sgp30_measure_iaq_blocking_read(&tvoc_ppb, &co2_eq_ppm);
        if (err == STATUS_OK)
        {
            NRF_LOG_INFO("tVOC  Concentration: %dppb\n", tvoc_ppb);
            NRF_LOG_INFO("CO2eq Concentration: %dppm\n", co2_eq_ppm);
        }
        else
        {
            NRF_LOG_INFO("error reading IAQ values\n");
        }
        NRF_LOG_FLUSH();
        /* Persist the current baseline every hour */
        if (++i % 3600 == 3599)
        {
            err = sgp30_get_iaq_baseline(&iaq_baseline);
            if (err == STATUS_OK)
            {
                /* IMPLEMENT: store baseline to presistent storage */
            }
        }

        /* The IAQ measurement must be triggered exactly once per second (SGP30)
         * to get accurate values.
         */
        sleep(1); // SGP30
    }
}

/** @} */
