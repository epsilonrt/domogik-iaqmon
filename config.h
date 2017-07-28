/**
 * Copyright © 2016 epsilonRT, All rights reserved.
 *
 * This software is governed by the CeCILL license under French law and
 * abiding by the rules of distribution of free software.  You can  use,
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * <http://www.cecill.info>.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 *
 * @file
 * @brief
 */
#ifndef _CONFIG_H_
#define _CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif

/* constants ================================================================ */
#define CFG_LOG_LEVEL           LOG_INFO
#define CFG_DAEMON_MAX_RESTARTS 100

#define CFG_QITH_PM  { 33, 58, 75, 91, 110 }
#define CFG_QITH_CO2 { 800, 1200, 2500, 5000, 10000 }
#define CFG_QITH_VOC { 200, 400, 800, 1600, 3200 }
#define CFG_QITH_HUM { {40, 60}, {35, 65}, {30, 70}, {20, 80}, {10, 90}}

#define CFG_XPL_POLL_RATE_MS    1000

#define CFG_XPL_VENDOR_ID       "domogik"
#define CFG_XPL_DEVICE_ID       "iaqmon"
#define CFG_XPL_CONFIG_FILENAME "domogik-iaqmon.xpl"
#define CFG_XPL_INSTANCE_ID     NULL // NULL for auto instance
#define CFG_XPL_DEVICE_VERSION  VERSION_SHORT // VERSION_SHORT is automatically defined in version-git.h from git describe

#define CFG_SENSOR_STAT_INTERVAL_NAME  "stat-interval"

// --- Qualité de l'air globale
#define CFG_SENSOR_AQI_DEVICE     "aqi"
#define CFG_SENSOR_AQI_TYPE       "aqi"

// Paramètres configurables par xPL
#define CFG_SENSOR_FLAG_NAME  "flag"

// --- Leds RGB
#define CFG_SENSOR_LED_DEVICE     "led"
#define CFG_SENSOR_LUM_TYPE       "slider"

// Paramètres configurables par xPL
#define CFG_SENSOR_LED_MAX_NAME  "led-max"

// --- Capteur de température et d'humidité
#define CFG_SENSOR_RHT_POLL_RATE  11
#define CFG_SENSOR_RHT_DEVICE     "rht"
#define CFG_SENSOR_TEMP_TYPE      "temp"
#define CFG_SENSOR_HUM_TYPE       "humidity"
#define CFG_SENSOR_HQI_TYPE       "humidity-qi"

// Paramètres configurables par xPL
#define CFG_SENSOR_TEMP_GAP_NAME  "temp-gap"
#define CFG_SENSOR_TEMP_ZERO_NAME "temp-zero"
#define CFG_SENSOR_HUM_GAP_NAME   "hum-gap"
#define CFG_SENSOR_HUM_ZERO_NAME  "hum-zero"

// --- Capteur de CO2 et TVOC
#define CFG_SENSOR_IAQ_POLL_RATE  19
#define CFG_SENSOR_IAQ_DEVICE     "iaq"
#define CFG_SENSOR_CO2_TYPE       "co2"
#define CFG_SENSOR_CO2_UNIT       "ppm"
#define CFG_SENSOR_CQI_TYPE       "co2-qi"
#define CFG_SENSOR_TVOC_TYPE      "tvoc"
#define CFG_SENSOR_TVOC_UNIT      "ppb"
#define CFG_SENSOR_TQI_TYPE       "tvoc-qi"

#define CFG_SENSOR_CO2_GAP_NAME   "co2-gap"
#define CFG_SENSOR_TVOC_GAP_NAME  "tvoc-gap"

// --- Capteur de particules fines
#define CFG_SENSOR_PM_POLL_RATE  23
#define CFG_SENSOR_PM_DEVICE     "pm"
#define CFG_SENSOR_PM_TYPE       "pm10"
#define CFG_SENSOR_PM_UNIT       "ug_m3"
#define CFG_SENSOR_PQI_TYPE      "pm10-qi"

// Paramètres configurables par xPL
#define CFG_SENSOR_PM_GAP_NAME   "pm10-gap"
#define CFG_SENSOR_PM_V1_NAME    "pm10-v1"
#define CFG_SENSOR_PM_V2_NAME    "pm10-v2"
#define CFG_SENSOR_PM_D1_NAME    "pm10-d1"
#define CFG_SENSOR_PM_D2_NAME    "pm10-d2"

/* default values =========================================================== */
// Options ligne de commande
#define CFG_DEFAULT_I2C_BUS     "/dev/i2c-1"
#define CFG_DEFAULT_RHT_ADDR    CHIPCAP2_I2CADDR
#define CFG_DEFAULT_IAQ_ADDR    IAQ_I2CADDR
#define CFG_DEFAULT_PM_ADDR     GP2I2C_I2CADDR

#define CFG_DEFAULT_LED_ADDR TLC59116_ADDR (0, 0, 0, 0)
/* Nombre et broches /OUT des leds RGB */
#define CFG_DEFAULT_LED_NOF_LEDS   4
#define CFG_DEFAULT_LED_LED1_RED   0
#define CFG_DEFAULT_LED_LED1_GREEN 1
#define CFG_DEFAULT_LED_LED1_BLUE  2
#define CFG_DEFAULT_LED_LED2_RED   9
#define CFG_DEFAULT_LED_LED2_GREEN 10
#define CFG_DEFAULT_LED_LED2_BLUE  11
#define CFG_DEFAULT_LED_LED3_RED   3
#define CFG_DEFAULT_LED_LED3_GREEN 4
#define CFG_DEFAULT_LED_LED3_BLUE  5
#define CFG_DEFAULT_LED_LED4_RED   6
#define CFG_DEFAULT_LED_LED4_GREEN 7
#define CFG_DEFAULT_LED_LED4_BLUE  8

#define CFG_DEFAULT_LED_MAX       512

// Paramètres xPL
#define CFG_DEFAULT_STAT_INTERVAL  300 // 0 pas de message stat périodique

#define CFG_DEFAULT_TEMP_GAP       0.1
#define CFG_DEFAULT_TEMP_ZERO      0.0

#define CFG_DEFAULT_HUM_GAP        5.0
#define CFG_DEFAULT_HUM_ZERO       0.0

#define CFG_DEFAULT_CO2_GAP        5
#define CFG_DEFAULT_TVOC_GAP       5

#define CFG_DEFAULT_PM_GAP        10
#define CFG_DEFAULT_PM_V1         300.0
#define CFG_DEFAULT_PM_D1         0.0
#define CFG_DEFAULT_PM_V2         3000.0
#define CFG_DEFAULT_PM_D2         400.0

//#define CFG_DEFAULT_FLAG      (IAQF_CO2 | IAQF_VOC | IAQF_PM | IAQF_HUM)
#define CFG_DEFAULT_FLAG      (IAQF_CO2 | IAQF_VOC | IAQF_PM)

/* build options ============================================================ */

/* conditionals options ====================================================== */

/* ========================================================================== */
#ifdef __cplusplus
}
#endif
#endif /* _CONFIG_H_ defined */
