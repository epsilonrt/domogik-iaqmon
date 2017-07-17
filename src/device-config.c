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
 * @brief Configurable device side
 */
#include <string.h>
#include <sysio/string.h>
#include "dmg-iaqmon.h"
#include "config.h"

/* constants ================================================================ */
/* public variables ========================================================= */
/* macros =================================================================== */
/* private variables ======================================================== */
/* private functions ======================================================== */
// --------------------------------------------------------------------------
//  It's best to put the logic for reading the device configuration
//  and parsing it into your code in a seperate function so it can
//  be used by your prvDeviceConfigChanged and your startup code that
//  will want to parse the same data after a setting file is loaded
void
prvDeviceSetConfig (gxPLDevice * device) {
  const char * str;

  str = gxPLDeviceConfigValueGet (device, CFG_SENSOR_STAT_INTERVAL_NAME);
  if (str) {
    long n;

    if (iStrToLong (str, &n, 0) == 0) {

      xCtx.ulStatInterval = (time_t) n;
      if (xCtx.ulStatInterval == 0) {
        
        // messages stat dévalidés
        xCtx.bSensorTrigEnabled = 1;
      }
      else {
        
        // période stat non nulle, on va envoyer un message stat
        xCtx.bSensorTrigEnabled = 0;
        xCtx.ulStatLastTime = 0;
      }
    }
  }

  str = gxPLDeviceConfigValueGet (device, CFG_SENSOR_TEMP_GAP_NAME);
  if (str) {
    double d;
    if (iStrToDouble (str, &d) == 0) {

      xCtx.xRhtGap.dTemp = d;
    }
  }

  str = gxPLDeviceConfigValueGet (device, CFG_SENSOR_TEMP_ZERO_NAME);
  if (str) {
    double d;
    if (iStrToDouble (str, &d) == 0) {

      xCtx.xRhtZero.dTemp = d;
    }
  }

  str = gxPLDeviceConfigValueGet (device, CFG_SENSOR_HUM_GAP_NAME);
  if (str) {
    double d;
    if (iStrToDouble (str, &d) == 0) {

      xCtx.xRhtGap.dHum = d;
    }
  }

  str = gxPLDeviceConfigValueGet (device, CFG_SENSOR_HUM_ZERO_NAME);
  if (str) {
    double d;
    if (iStrToDouble (str, &d) == 0) {

      xCtx.xRhtZero.dHum = d;
    }
  }

  str = gxPLDeviceConfigValueGet (device, CFG_SENSOR_CO2_GAP_NAME);
  if (str) {
    long n;

    if (iStrToLong (str, &n, 0) == 0) {

      if ((n <= UINT16_MAX) && (n > 0)) {

        xCtx.xIaqGap.usCo2 = (uint16_t) n;
      }
    }
  }

  str = gxPLDeviceConfigValueGet (device, CFG_SENSOR_TVOC_GAP_NAME);
  if (str) {
    long n;

    if (iStrToLong (str, &n, 0) == 0) {

      if ((n <= UINT16_MAX) && (n > 0)) {

        xCtx.xIaqGap.usTvoc = (uint16_t) n;
      }
    }
  }

  str = gxPLDeviceConfigValueGet (device, CFG_SENSOR_PM_GAP_NAME);
  if (str) {
    long n;
    if (iStrToLong (str, &n, 0) == 0) {

      xCtx.iPmGap = n;
    }
  }

  str = gxPLDeviceConfigValueGet (device, CFG_SENSOR_PM_V1_NAME);
  if (str) {
    double d;
    if (iStrToDouble (str, &d) == 0) {

      xCtx.xPmSetting.dV1 = d;
      xCtx.bPmSettingChanged = 1;
    }
  }

  str = gxPLDeviceConfigValueGet (device, CFG_SENSOR_PM_V2_NAME);
  if (str) {
    double d;
    if (iStrToDouble (str, &d) == 0) {

      xCtx.xPmSetting.dV2 = d;
      xCtx.bPmSettingChanged = 1;
    }
  }

  str = gxPLDeviceConfigValueGet (device, CFG_SENSOR_PM_D1_NAME);
  if (str) {
    double d;
    if (iStrToDouble (str, &d) == 0) {

      xCtx.xPmSetting.dD1 = d;
      xCtx.bPmSettingChanged = 1;
    }
  }

  str = gxPLDeviceConfigValueGet (device, CFG_SENSOR_PM_D2_NAME);
  if (str) {
    double d;
    
    if (iStrToDouble (str, &d) == 0) {

      xCtx.xPmSetting.dD2 = d;
      xCtx.bPmSettingChanged = 1;
    }
  }
}

// --------------------------------------------------------------------------
//  Handle a change to the device device configuration
static void
prvDeviceConfigChanged (gxPLDevice * device, void * udata) {

  gxPLMessageSourceInstanceIdSet (xCtx.xSensorMsg, gxPLDeviceInstanceId (device));

  // Read setting items for device and install
  prvDeviceSetConfig (device);
}

/* internal public functions ================================================ */
// -----------------------------------------------------------------------------
// Create xPL application and device
gxPLDevice *
xDeviceCreate (gxPLSetting * setting) {
  gxPLApplication * app;
  gxPLDevice * device;

  // opens the xPL network
  app = gxPLAppOpen (setting);
  if (app == NULL) {

    return NULL;
  }

  // Initialize sensor device
  // Create a device for us
  // Create a configurable device and set our application version
  device = gxPLAppAddConfigurableDevice (app, CFG_XPL_VENDOR_ID,
                                         CFG_XPL_DEVICE_ID,
                                         gxPLConfigPath (CFG_XPL_CONFIG_FILENAME));
  if (device == NULL) {

    return NULL;
  }

  gxPLDeviceVersionSet (device, CFG_XPL_DEVICE_VERSION);

  // If the configuration was not reloaded, then this is our first time and
  // we need to define what the configurables are and what the default values
  // should be.
  if (gxPLDeviceIsConfigured (device) == false) {

    // Define configurable items and give it a default
    gxPLDeviceConfigItemAdd (device, CFG_SENSOR_STAT_INTERVAL_NAME, gxPLConfigReconf, 1);
    gxPLDeviceConfigValueSet (device, CFG_SENSOR_STAT_INTERVAL_NAME,
                              gxPLLongToStr (CFG_DEFAULT_STAT_INTERVAL));

    gxPLDeviceConfigItemAdd (device, CFG_SENSOR_TEMP_GAP_NAME, gxPLConfigReconf, 1);
    gxPLDeviceConfigValueSet (device, CFG_SENSOR_TEMP_GAP_NAME,
                              gxPLDoubleToStr (CFG_DEFAULT_TEMP_GAP, 1));

    gxPLDeviceConfigItemAdd (device, CFG_SENSOR_TEMP_ZERO_NAME, gxPLConfigReconf, 1);
    gxPLDeviceConfigValueSet (device, CFG_SENSOR_TEMP_ZERO_NAME,
                              gxPLDoubleToStr (CFG_DEFAULT_TEMP_ZERO, 1));

    gxPLDeviceConfigItemAdd (device, CFG_SENSOR_HUM_GAP_NAME, gxPLConfigReconf, 1);
    gxPLDeviceConfigValueSet (device, CFG_SENSOR_HUM_GAP_NAME,
                              gxPLDoubleToStr (CFG_DEFAULT_HUM_GAP, 1));

    gxPLDeviceConfigItemAdd (device, CFG_SENSOR_HUM_ZERO_NAME, gxPLConfigReconf, 1);
    gxPLDeviceConfigValueSet (device, CFG_SENSOR_HUM_ZERO_NAME,
                              gxPLDoubleToStr (CFG_DEFAULT_HUM_ZERO, 1));

    gxPLDeviceConfigItemAdd (device, CFG_SENSOR_CO2_GAP_NAME, gxPLConfigReconf, 1);
    gxPLDeviceConfigValueSet (device, CFG_SENSOR_CO2_GAP_NAME,
                              gxPLLongToStr (CFG_DEFAULT_CO2_GAP));

    gxPLDeviceConfigItemAdd (device, CFG_SENSOR_TVOC_GAP_NAME, gxPLConfigReconf, 1);
    gxPLDeviceConfigValueSet (device, CFG_SENSOR_TVOC_GAP_NAME,
                              gxPLLongToStr (CFG_DEFAULT_TVOC_GAP));

    gxPLDeviceConfigItemAdd (device, CFG_SENSOR_PM_GAP_NAME, gxPLConfigReconf, 1);
    gxPLDeviceConfigValueSet (device, CFG_SENSOR_PM_GAP_NAME,
                              gxPLLongToStr (CFG_DEFAULT_PM_GAP));

    gxPLDeviceConfigItemAdd (device, CFG_SENSOR_PM_V1_NAME, gxPLConfigReconf, 1);
    gxPLDeviceConfigValueSet (device, CFG_SENSOR_PM_V1_NAME,
                              gxPLDoubleToStr (CFG_DEFAULT_PM_V1, 1));

    gxPLDeviceConfigItemAdd (device, CFG_SENSOR_PM_V2_NAME, gxPLConfigReconf, 1);
    gxPLDeviceConfigValueSet (device, CFG_SENSOR_PM_V2_NAME,
                              gxPLDoubleToStr (CFG_DEFAULT_PM_V2, 1));

    gxPLDeviceConfigItemAdd (device, CFG_SENSOR_PM_D1_NAME, gxPLConfigReconf, 1);
    gxPLDeviceConfigValueSet (device, CFG_SENSOR_PM_D1_NAME,
                              gxPLDoubleToStr (CFG_DEFAULT_PM_D1, 1));

    gxPLDeviceConfigItemAdd (device, CFG_SENSOR_PM_D2_NAME, gxPLConfigReconf, 1);
    gxPLDeviceConfigValueSet (device, CFG_SENSOR_PM_D2_NAME,
                              gxPLDoubleToStr (CFG_DEFAULT_PM_D2, 1));

  }

  // Create a sensor.basic message conforming to http://xplproject.org.uk/wiki/Schema_-_SENSOR.html
  xCtx.xSensorMsg = gxPLDeviceMessageNew (device, gxPLMessageTrigger);
  assert (xCtx.xSensorMsg);

  // Setting up the message
  gxPLMessageBroadcastSet (xCtx.xSensorMsg, true);
  gxPLMessageSchemaSet (xCtx.xSensorMsg, "sensor", "basic");

  // Parse the device configurables into a form this program
  // can use (whether we read a setting or not)
  prvDeviceSetConfig (device);

  // Add a device change listener we'll use to pick up a new gap
  gxPLDeviceConfigListenerAdd (device, prvDeviceConfigChanged, NULL);

  return device;
}

/* ========================================================================== */
