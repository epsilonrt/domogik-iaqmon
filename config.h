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

#define CFG_XPL_POLL_RATE_MS    1000

#define CFG_XPL_VENDOR_ID       "domogik"
#define CFG_XPL_DEVICE_ID       "iaqmon"
#define CFG_XPL_CONFIG_FILENAME "domogik-iaqmon.xpl"
#define CFG_XPL_INSTANCE_ID     NULL // NULL for auto instance
#define CFG_XPL_DEVICE_VERSION  VERSION_SHORT // VERSION_SHORT is automatically defined in version-git.h from git describe

#define CFG_SENSOR_STAT_INTERVAL_NAME  "stat-interval"

// --- Capteur de température et d'humidité
#define CFG_SENSOR_RHT_DEVICE     "rht"
#define CFG_SENSOR_TEMP_TYPE      "temp"
#define CFG_SENSOR_HUM_TYPE       "humidity"

// Paramètres configurables par xPL
#define CFG_SENSOR_TEMP_GAP_NAME  "temp-gap"
#define CFG_SENSOR_TEMP_ZERO_NAME "temp-zero"
#define CFG_SENSOR_HUM_GAP_NAME   "hum-gap"
#define CFG_SENSOR_HUM_ZERO_NAME  "hum-zero"

// --- Capteur de CO2 et TVOC
#define CFG_SENSOR_IAQ_DEVICE     "iaq"
#define CFG_SENSOR_CO2_TYPE       "co2"
#define CFG_SENSOR_CO2_UNIT       "ppm"
#define CFG_SENSOR_TVOC_TYPE      "tvoc"
#define CFG_SENSOR_TVOC_UNIT      "ppb"

// Paramètres configurables par xPL
#define CFG_SENSOR_CO2_GAP_NAME   "co2-gap"
#define CFG_SENSOR_TVOC_GAP_NAME  "tvoc-gap"

/* default values =========================================================== */
// Options ligne de commande
#define CFG_DEFAULT_I2C_BUS     "/dev/i2c-1"
#define CFG_DEFAULT_RHT_ADDR    CHIPCAP2_I2CADDR
#define CFG_DEFAULT_IAQ_ADDR    IAQ_I2CADDR

// Paramètres xPL
#define CFG_DEFAULT_STAT_INTERVAL  300 // 0 pas de message stat périodique
#define CFG_DEFAULT_TEMP_GAP       0.1
#define CFG_DEFAULT_TEMP_ZERO      0.0
#define CFG_DEFAULT_HUM_GAP        5.0
#define CFG_DEFAULT_HUM_ZERO       0.0
#define CFG_DEFAULT_CO2_GAP        5
#define CFG_DEFAULT_TVOC_GAP       5

/* build options ============================================================ */

/* conditionals options ====================================================== */

/* ========================================================================== */
#ifdef __cplusplus
}
#endif
#endif /* _CONFIG_H_ defined */
