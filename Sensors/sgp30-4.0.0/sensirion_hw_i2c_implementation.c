/*
 * Copyright (c) 2018, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <nrf_delay.h>
#include <nrf_drv_twi.h>
#include <stdio.h>

#include "sensirion_arch_config.h"
#include "sensirion_i2c.h"

/**
 * Nordic specific configuration. Change the pin numbers if you use other pins
 * than defined below.
 */
#define SENSIRION_SDA_PIN 26
#define SENSIRION_SCL_PIN 27

/**
 * Create new TWI instance. You may also use a different interface. In this case, please adapt
 * the code below.
 */
static const nrf_drv_twi_t i2c_instance = NRF_DRV_TWI_INSTANCE(0);

/**
 * Initialize all hard- and software components that are needed for the I2C
 * communication.
 */
void sensirion_i2c_init()
{
    s8 err;
    const nrf_drv_twi_config_t i2c_instance_config = {
        .scl = SENSIRION_SCL_PIN,
        .sda = SENSIRION_SDA_PIN,
        .frequency = NRF_TWI_FREQ_100K,
        .interrupt_priority = 0};
    /* initiate TWI instance */
    err = nrf_drv_twi_init(&i2c_instance, &i2c_instance_config, NULL, NULL);
    if (err)
    {
        /* Could be omitted if the prototyp is changed to non-void or an error flag is introduced */
        printf("Error %d: Initialization of I2C connection failed!\n", err);
    }
    /* enable TWI instance */
    nrf_drv_twi_enable(&i2c_instance);
    return;
}

/**
 * Execute one read transaction on the I2C bus, reading a given number of bytes.
 * If the device does not acknowledge the read command, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to read from
 * @param data    pointer to the buffer where the data is to be stored
 * @param count   number of bytes to read from I2C and store in the buffer
 * @returns 0 on success, error code otherwise
 *
 * error codes:  3 -> error detected by hardware (internal error)
 *              17 -> driver not ready for new transfer (busy)
 */
s8 sensirion_i2c_read(u8 address, u8 *data, u16 count)
{
    s8 err = nrf_drv_twi_rx(&i2c_instance, address, data, (u8)count);
    return err;
}

/**
 * Execute one write transaction on the I2C bus, sending a given number of bytes.
 * The bytes in the supplied buffer must be sent to the given address. If the
 * slave device does not acknowledge any of the bytes, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to write to
 * @param data    pointer to the buffer containing the data to write
 * @param count   number of bytes to read from the buffer and send over I2C
 * @returns 0 on success, error code otherwise
 *
 * error codes:  3 -> error detected by hardware (internal error)
 *              17 -> driver not ready for new transfer (busy)
 */
s8 sensirion_i2c_write(u8 address, const u8 *data, u16 count)
{
    s8 err = nrf_drv_twi_tx(&i2c_instance, address, data, (u8)count, false);
    return err;
}

/**
 * Sleep for a given number of microseconds. The function should delay the
 * execution for at least the given time, but may also sleep longer.
 *
 * @param useconds the sleep time in microseconds
 */
void sensirion_sleep_usec(u32 useconds)
{
    nrf_delay_us(useconds);
}
