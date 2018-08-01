/*******************************************************************************
 * Copyright (C) 2018 Maxim Integrated Products, Inc., All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 *******************************************************************************
 */

#include "MAX17055.h"

//******************************************************************************
MAX17055::MAX17055(I2C &i2c, int address) :
    i2c(i2c),
    addr(address)
{
}

//******************************************************************************
MAX17055::~MAX17055()
{
}

//******************************************************************************
void MAX17055::init(float r_sense)
{
    this->r_sense = r_sense;
    i_lsb = MAX17055_I_LSB_UV / r_sense;
    i_min_max_lsb = MAX17055_I_MAX_MIN_LSB_MV / r_sense;
}

//******************************************************************************
int MAX17055::status(int *value)
{
    char buf[2];

    if (readReg(MAX17055::STATUS, buf, 2)) {
        return MAX17055_ERROR;
    }

    *value = (unsigned int)((buf[1] << 8) | buf[0]);

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::v_cell(int *value)
{
    char buf[2];

    if (readReg(MAX17055::V_CELL, buf, 2)) {
        return MAX17055_ERROR;
    }

    *value = (unsigned int)((buf[1] << 8) | buf[0]);

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::v_cell(float *value)
{
    int v;

    if (v_cell(&v)) {
        return MAX17055_ERROR;
    }

    *value = v * MAX17055_V_LSB_MV;

    return MAX17055_NO_ERROR;
}

/** ************************************************************************ **/
/** ************************ added by gv 7/8/2018 ************************** **/
/** ************************************************************************ **/
int MAX17055::soc1(int *value)
{
    char buf[2];
    
    if (readReg(MAX17055::REP_SOC,buf,2))
    {
        return MAX17055_ERROR;    
    }   
      
    *value = (unsigned int)((buf[1]<<8) | buf[0]);
    
    return MAX17055_NO_ERROR;
} 

int MAX17055::soc2(int *valueLow, int *valueHigh)
{

    char buf[2];

    if (readReg(MAX17055::REP_SOC, buf, 2)) {
        return MAX17055_ERROR;
    }

    *valueHigh = (unsigned char)buf[1];
    *valueLow = (unsigned char)buf[0];
    return MAX17055_NO_ERROR;
    
}

int MAX17055::reportSOC(int *valueHigh)
{

    char buf[2];

    if (readReg(MAX17055::REP_SOC, buf, 2)) {
        return MAX17055_ERROR;
    }

    *valueHigh = (unsigned char)buf[1];
    
    return MAX17055_NO_ERROR;
    
}

int MAX17055::tte1(int *value)
{
    char buf[2];
    
    if (readReg(MAX17055::TTE,buf,2))
    {
        return MAX17055_ERROR;    
    }   
      
    *value = (unsigned int)((buf[1]<<8) | buf[0]);
    
    return MAX17055_NO_ERROR;
} 

int MAX17055::tte2(int *valueLow, int *valueHigh)
{

    char buf[2];

    if (readReg(MAX17055::TTE, buf, 2)) {
        return MAX17055_ERROR;
    }

    *valueHigh = (unsigned char)buf[1];
    *valueLow = (unsigned char)buf[0];
    
    return MAX17055_NO_ERROR;
}


/** ************************************************************************ **/
/** ************************************************************************ **/

//******************************************************************************
int MAX17055::avg_v_cell(int *value)
{
    char buf[2];

    if (readReg(MAX17055::AVG_V_CELL, buf, 2)) {
        return MAX17055_ERROR;
    }

    *value = (unsigned int)((buf[1] << 8) | buf[0]);

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::avg_v_cell(float *value)
{
    int avc;

    if (avg_v_cell(&avc)) {
        return MAX17055_ERROR;
    }

    *value = avc * MAX17055_V_LSB_MV;

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::max_min_volt(int *max, int *min)
{
    char buf[2];

    if (readReg(MAX17055::MAX_MIN_VOLT, buf, 2)) {
        return MAX17055_ERROR;
    }

    *max = (unsigned char)buf[1];
    *min = (unsigned char)buf[0];

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::max_min_volt(float *max, float *min)
{
    int v_max, v_min;

    if (max_min_volt(&v_max, &v_min)) {
        return MAX17055_ERROR;
    }

    *max = v_max * MAX17055_V_MAX_MIN_LSB_MV;
    *min = v_min * MAX17055_V_MAX_MIN_LSB_MV;

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::current(int *value)
{
    char buf[2];

    if (readReg(MAX17055::CURRENT, buf, 2)) {
        return MAX17055_ERROR;
    }

    *value = (short int)((buf[1] << 8) | buf[0]);

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::current(float *value)
{
    int i;

    if (current(&i)) {
        return MAX17055_ERROR;
    }

    *value = i * i_lsb;

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::avg_current(int *value)
{
    char buf[2];

    if (readReg(MAX17055::AVG_CURRENT, buf, 2)) {
        return MAX17055_ERROR;
    }

    *value = (short int)((buf[1] << 8) | buf[0]);

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::avg_current(float *value)
{
    int i_a;

    if (avg_current(&i_a)) {
        return MAX17055_ERROR;
    }

    *value = i_a * i_lsb;

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::max_min_curr(int *max, int *min)
{
    char buf[2];

    if (readReg(MAX17055::MAX_MIN_CURR, buf, 2)) {
        return MAX17055_ERROR;
    }

    *max = (signed char)buf[1];
    *min = (signed char)buf[0];

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::max_min_curr(float *max, float *min)
{
    int i_max, i_min;

    if (max_min_curr(&i_max, &i_min)) {
        return MAX17055_ERROR;
    }

    *max = i_max * i_min_max_lsb;
    *min = i_min * i_min_max_lsb;

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::temp(int *value)
{
    char buf[2];

    if (readReg(MAX17055::TEMP, buf, 2)) {
        return MAX17055_ERROR;
    }

    *value = (short int)((buf[1] << 8) | buf[0]);

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::temp(float *value)
{
    int t;

    if (temp(&t)) {
        return MAX17055_ERROR;
    }

    *value = t * (1.0f / 256);

    return MAX17055_NO_ERROR;
}


//******************************************************************************
int MAX17055::avg_ta(int *value)
{
    char buf[2];

    if (readReg(MAX17055::AVG_TA, buf, 2)) {
        return MAX17055_ERROR;
    }

    *value = (short int)((buf[1] << 8) | buf[0]);

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::avg_ta(float *value)
{
    int ta;

    if (avg_ta(&ta)) {
        return MAX17055_ERROR;
    }

    *value = ta * (1.0f / 256);

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::max_min_temp(int *max, int *min)
{
    char buf[2];

    if (readReg(MAX17055::MAX_MIN_TEMP, buf, 2)) {
        return MAX17055_ERROR;
    }

    *max = (signed char)buf[1];
    *min = (signed char)buf[0];

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::max_min_temp(float *max, float *min)
{
    int t_max, t_min;

    if (max_min_temp(&t_max, &t_min)) {
        return MAX17055_ERROR;
    }

    *max = t_max;
    *min = t_min;

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::writeReg(reg_t reg, char value, bool verify)
{
    char buf[] = { (char)reg, value };

    if (i2c.write(addr, buf, sizeof(buf))) {
        return MAX17055_ERROR;
    }

    return MAX17055_NO_ERROR;
}

/** ************************************************************************ **/
/** ************ Special to write config to use Thermistor ***************** **/
/** ************************************************************************ **/
int MAX17055::writeConfig()
{
    char cmd[3];
    cmd[0] = MAX17055::CONFIG;
    cmd[1] = 0xA2;
    cmd[2] = 0x10;    
    
    
   if (i2c.write(0x6C, cmd, 3))
   {
        return MAX17055_ERROR;    
   }
   Thread::wait(1);
   return MAX17055_NO_ERROR; 
}



//******************************************************************************
int MAX17055::writeReg(reg_t reg, uint16_t value, bool verify)
{
    char wbuf[3];
    char rbuf[2];
    int attempt;

    wbuf[0] = reg;
    wbuf[1] = value;
    wbuf[2] = value >> 8;

    attempt = 0;

    do {
        if (i2c.write(addr, wbuf, 3)) return MAX17055_ERROR;

        if (!verify) break;

        Thread::wait(1);

        if (i2c.read(addr, rbuf, 2)) return MAX17055_ERROR;

    } while ((((rbuf[1] << 8) | rbuf[0]) != value) && (attempt++ < 3));

    if (attempt == 3) return MAX17055_ERROR;

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::readReg(reg_t reg, char *value)
{
    char buf[] = { (char)reg };

    if (i2c.write(addr, buf, sizeof(buf))) {
        return MAX17055_ERROR;
    }

    if (i2c.read(addr, value, 1)) {
        return MAX17055_ERROR;
    }

    return MAX17055_NO_ERROR;
}

//******************************************************************************
int MAX17055::readReg(reg_t reg, char *buf, int len)
{
    *buf = (char)reg;

    if (i2c.write(addr, buf, 1)) {
        return MAX17055_ERROR;
    }

    if (i2c.read(addr, buf, len)) {
        return MAX17055_ERROR;
    }

    return MAX17055_NO_ERROR;
}
