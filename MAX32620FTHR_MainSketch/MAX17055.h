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

#ifndef _MAX17055_H_
#define _MAX17055_H_

#include "mbed.h"

#define MAX17055_I2C_ADDRESS   0x6C

#define MAX17055_NO_ERROR 0
#define MAX17055_ERROR    -1

#define MAX17055_V_LSB_MV         7.8125E-5f
#define MAX17055_V_MAX_MIN_LSB_MV 2.0E-2f
#define MAX17055_I_LSB_UV         1.5625E-3f
#define MAX17055_I_MAX_MIN_LSB_MV 0.0004f

/**
 * @brief MAX17055 7µA 1-Cell Fuel Gauge with ModelGauge m5 EZ
 *
 * @details The MAX17055 is a low 7μA operating current fuel gauge which
 * implements Maxim ModelGauge™ m5 EZ algorithm.  ModelGauge m5 EZ makes
 * fuel gauge implementation easy by eliminating battery characterization
 * requirements and simplifying host software interaction.
 * The ModelGauge m5 EZ robust algorithm provides tolerance against battery
 * diversity for most lithium batteries and applications.
 * <br>https://www.maximintegrated.com/en/products/power/battery-management/MAX17055.html
 *
 * @code
 * #include "mbed.h"
 * #include "MAX17055.h"
 *
 * // I2C Master 2
 * I2C i2c(I2C2_SDA, I2C2_SCL);
 *
 * MAX17055 max17055(i2c);
 *
 * int main()
 * {
 *     int status;
 *     float f_value;
 *
 *     // Set sense resistor value
 *     max17055.init(0.05f);
 *
 *     // Print status
 *     max17055.status(&status);
 *     printf("MAX17055 status: %04X\r\n", (uint16_t)status);
 *
 *     for (;;) {
 *         // Print formatted voltage, current and temperature values
 *         max17055.v_cell(&f_value);
 *         printf("%6.3fV ", f_value);
 *         max17055.current(&f_value);
 *         printf("%6.3fI ", f_value);
 *         max17055.temp(&f_value);
 *         printf("%6.3fC ", f_value);
 *         printf("\r\n");
 *
 *         Thread::wait(3000);
 *     }
 * }
 * @endcode
 */

class MAX17055
{
public:

    /**
     * @brief   Register Address
     * @details MAX17055 register addresses
     */
    typedef enum {
        STATUS = 0x00,
        V_ALRT_TH,
        T_ALRT_TH,
        S_ALRT_TH,
        AT_RATE,
        REP_CAP,
        REP_SOC,
        AGE,
        TEMP,
        V_CELL,
        CURRENT,
        AVG_CURRENT,
        Q_RESIDUAL,
        MIX_SOC,
        AV_SOC,
        MIX_CAP,
        FULL_CAP_REP,
        TTE,
        QR_TABLE_00,
        FULL_SOC_THR,
        R_CELL,
        AVG_TA = 0x16,
        CYCLES,
        DESIGN_CAP,
        AVG_V_CELL,
        MAX_MIN_TEMP,
        MAX_MIN_VOLT,
        MAX_MIN_CURR,
        CONFIG,
        I_CHG_TERM,
        AV_CAP,
        TTF,
        DEV_NAME,
        QR_TABLE_10,
        FULL_CAP_NOM,
        AIN = 0x27,
        LEARN_CFG,
        FILTER_CFG,
        RELAX_CFG,
        MISC_CFG,
        T_GAIN,
        T_OFF,
        C_GAIN,
        C_OFF,
        QR_TABLE_20 = 0x32,
        DIE_TEMP = 0x34,
        FULL_CAP,
        R_COMP0 = 0x38,
        TEMP_CO,
        V_EMPTY,
        F_STAT = 0x3D,
        TIMER,
        SHDN_TIMER,
        USER_MEM1,
        QR_TABLE_30 = 0x42,
        R_GAIN,
        DQ_ACC = 0x45,
        DP_ACC,
        CONVG_CFG = 0x49,
        VF_REM_CAP,
        QH = 0x4D,
        STATUS_2 = 0xB0,
        POWER,
        ID_USER_MEM2,
        AVG_POWER,
        I_ALRT_TH,
        TTF_CFG,
        CV_MIX_CAP,
        CV_HALF_TIME,
        CG_TEMP_CO,
        CURVE,
        HIB_CFG,
        CONFIG2,
        V_RIPPLE,
        RIPPLE_CFG,
        TIMER_H,
        R_SENSE_USER_MEM3 = 0xD0,
        SC_OCV_LIM,
        SOC_HOLD,
        MAX_PEAK_POWER,
        SUS_PEAK_POWER,
        PACK_RESISTANCE,
        SYS_RESISTANCE,
        MIN_SYS_VOLTAGE,
        MPP_CURRENT,
        SPP_CURRENT,
        MODEL_CFG,
        AT_Q_RESIDUAL,
        AT_TTE,
        AT_AV_SOC,
        AT_AV_CA
    } reg_t;

    /**
     * MAX17055 constructor
     *
     * @param i2c I2C object to use.
     * @param address Slave Address of the device.
     */
    MAX17055(I2C &i2c, int address = MAX17055_I2C_ADDRESS);

    /**
     * MAX17055 destructor
     */
    ~MAX17055();

    /**
     * @brief   Initialize driver state
     * @details Initialize driver with supplied sense resistor value.
     * @param   value The sense resistor value in ohms
     * @returns 0 if no errors, -1 if error
     */
    void init(float r_sense);

    /**
     * @brief   Read status register
     * @details Read status register.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int status(int *value);

    /**
     * @brief   Read VCell register
     * @details Read VCell register reports the voltage measured between
     * BATT and CSP.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int v_cell(int *value);

    /**
     * @brief   Read VCell register
     * @details The VCell register reports the voltage measured between
     * BATT and CSP.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int v_cell(float *value);

    /**
     * @brief   Read AvgVCell register
     * @details The AvgVCell register reports an average of the VCell
     * register readings.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int avg_v_cell(int *value);

    /**
     * @brief   Read AvgVCell register
     * @details The AvgVCell register reports an average of the VCell
     * register readings.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int avg_v_cell(float *value);

    /**
     * @brief   Read MaxMinVolt register
     * @details The MaxMinVolt register maintains the maximum and minimum
     * of VCell register values since device reset.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int max_min_volt(int *max, int *min);

    /**
     * @brief   Read MaxMinVolt register
     * @details The MaxMinVolt register maintains the maximum and minimum
     * of VCell register values since device reset.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int max_min_volt(float *max, float *min);

    /**
     * @brief   Read Current register
     * @details The Current register reports the voltage between the
     * CSP and CSN pins.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int current(int *value);

    /**
     * @brief   Read Current register
     * @details The Current register reports the voltage between the
     * CSP and CSN pins.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int current(float *value);

    /**
     * @brief   Read AvgCurrent register
     * @details The AvgCurrent register reports an average of Current
     * register readings.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int avg_current(int *value);

    /**
     * @brief   Read AvgCurrent register
     * @details The AvgCurrent register reports an average of Current
     * register readings.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int avg_current(float *value);

    /**
     * @brief   Read MinMaxCurr register
     * @details The MaxMinCurr register maintains the maximum and
     * minimum Current register values since the last IC reset
     * or until cleared by host software.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int max_min_curr(int *max, int *min);

    /**
     * @brief   Read MinMaxCurrent register
     * @details The MaxMinCurr register maintains the maximum and
     * minimum Current register values since the last IC reset
     * or until cleared by host software.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int max_min_curr(float *max, float *min);

    /**
     * @brief   Read Temp register
     * @details The Temp register provides the temperature measured
     * by the thermistor or die temperature.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int temp(int *value);

    /**
     * @brief   Read Temp register
     * @details The Temp register provides the temperature measured
     * by the thermistor or die temperature.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int temp(float *value);

    /**
     * @brief   Read AvgTA register
     * @details The AvgTA register reports an average of the readings
     * from the Temp register.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int avg_ta(int *value);

    /**
     * @brief   Read AvgTA register
     * @details The AvgTA register reports an average of the readings
     * from the Temp register.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int avg_ta(float *value);

    /**
     * @brief   Read MaxMinTemp register
     * @details The MaxMinTemp register maintains the maximum and
     * minimum Temp register values since the last fuel-gauge reset
     * or until cleared by host software.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int max_min_temp(int *max, int *min);

    /**
     * @brief   Read MaxMinTemp register
     * @details The MaxMinTemp register maintains the maximum and
     * minimum Temp register values since the last fuel-gauge reset
     * or until cleared by host software.
     * @param   value The location to store value read
     * @returns 0 if no errors, -1 if error
     */
    int max_min_temp(float *max, float *min);

    /**
     * @brief   Write 8-Bit Register
     * @details Writes the given value to the specified register.
     * @param   reg The register to be written
     * @param   value The data to be written
     * @param   verify Verify data after write
     * @returns 0 if no errors, -1 if error.
     */
    int writeReg(reg_t reg, char value, bool verify = false);

    /**
     * @brief   Write 16-Bit Register
     * @details Writes the given value to the specified register.
     * @param   reg The register to be written
     * @param   value The data to be written
     * @param   verify Verify data after write
     * @returns 0 if no errors, -1 if error.
     */
    int writeReg(reg_t reg, uint16_t value, bool verify = false);

    /**
     * @brief   Read 8-Bit Register
     * @details Reads from the specified register
     * @param   reg The register to be read
     * @param   value Pointer for where to store the data
     * @returns 0 if no errors, -1 if error.
     */
    int readReg(reg_t reg, char *value);

    /**
     * @brief   Read 8-Bit Register(s)
     * @details Reads len bytes starting from the specified register
     * @param   reg The register to be read
     * @param   value Pointer for where to store the data
     * @param   len Number of bytes to read
     * @returns 0 if no errors, -1 if error.
     */
    int readReg(reg_t reg, char *buf, int len);

    /**
     * @brief   Read 16-Bit Register
     * @details Reads from the specified register
     * @param   reg The register to be read
     * @param   value Pointer for where to store the data
     * @returns 0 if no errors, -1 if error.
     */
    int readReg16(reg_t reg, int *value);
    
//** ***************************************************************** **/
//** *********************** added by gv 7/8/2018 ******************** **/
//** ***************************************************************** **/
    int MAX17055::soc1(int *value);
    int MAX17055::soc2(int *valueLow, int *valueHigh);
    
    int MAX17055::reportSOC(int *valueHigh);
    
    int MAX17055::tte1(int *value);
    int MAX17055::tte2(int *valueLow, int *valueHigh);
    int MAX17055::writeConfig(void);
    
private:
    I2C &i2c;
    int addr;

    float r_sense;
    float i_lsb;
    float i_min_max_lsb;
};

#endif
