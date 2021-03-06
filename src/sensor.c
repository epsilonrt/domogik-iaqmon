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
#include <math.h>
#include <string.h>
#include "dmg-iaqmon.h"
#include "config.h"

/* private functions ======================================================== */

/* -----------------------------------------------------------------------------
 * Démare la mesure de temp/hum si elle n'est pas démarrée
 */
static int
priRhtStart (void) {
  int ret = 0;

  if (xCtx.bRhtStarted == 0) {

    ret = iHih6130Start (xCtx.xRhtSensor);
    if (ret == 0) {

      xCtx.bRhtStarted = 1;
    }
  }

  return ret;
}

/* -----------------------------------------------------------------------------
 * Effectue la mesure de température et d'humidité :
 * - Démare la mesure si elle n'est pas démarrée
 */
static int
priRhtRead (xHih6130Data * xHihCurrent) {
  int ret;

  ret = iHih6130Read (xCtx.xRhtSensor, xHihCurrent);
  if (ret < 0) {

    return ret;
  }
  else if (ret == 0) {

    xCtx.bRhtStarted = 0;
    // Correction de zéro
    xHihCurrent->dTemp -= xCtx.xRhtZero.dTemp;
    xHihCurrent->dHum -= xCtx.xRhtZero.dHum;
  }

  return ret;
}

/* -----------------------------------------------------------------------------
 * Effectue les mesures et transmets les messages correspondants si nécessaire
 */
static int
priSendCurrentValue (gxPLDevice * device, gxPLMessageType msgtype) {
  int ret;

  time_t t = time (NULL);

  if (xCtx.xRhtSensor) {

    // Mesure capteur HIH6130/ChipCap2
    ret = priRhtRead (&xCtx.xRhtCurrent);

    if (ret < 0) {

      vLog (LOG_ERR, "iHih6130Read() returns %d, Unable to read RHT sensor !", ret);
    }
    else if (ret == 0) {

      if (msgtype == gxPLMessageTrigger) {

        if (fabs (xCtx.xRhtCurrent.dTemp - xCtx.xRhtLastTx.dTemp) >= xCtx.xRhtGap.dTemp) {

          xCtx.bTempRequest = 1;
        }
        if (fabs (xCtx.xRhtCurrent.dHum - xCtx.xRhtLastTx.dHum) >= xCtx.xRhtGap.dHum) {

          xCtx.bHumRequest = 1;
        }
      }

      priRhtStart();
    }
  }

  if (xCtx.xIaqSensor) {

    // Mesure capteur IAQ
    if ( (t - xCtx.ulIaqLastTime) > CFG_SENSOR_IAQ_POLL_RATE) {

      ret = iIaqRead (xCtx.xIaqSensor, &xCtx.xIaqCurrent);
      if (ret < 0) {

        vLog (LOG_ERR, "iIaqRead() returns %d, Unable to read IAQ sensor !", ret);
      }
      else if (ret == 0) {

        xCtx.ulIaqLastTime = t;
        if (msgtype == gxPLMessageTrigger) {

          if (abs ( (int) xCtx.xIaqCurrent.usCo2 - (int) xCtx.xIaqLastTx.usCo2) >= (int) xCtx.xIaqGap.usCo2) {

            xCtx.bCo2Request = 1;
          }
          if (abs ( (int) xCtx.xIaqCurrent.usTvoc - (int) xCtx.xIaqLastTx.usTvoc) >= (int) xCtx.xIaqGap.usTvoc) {

            xCtx.bTvocRequest = 1;
          }
        }
      }
    }
  }

  if (xCtx.xPmSensor) {

    // Mesure capteur GP2
    if (xCtx.bPmSettingChanged) {

      vGp2SetSetting (xCtx.xPmSensor, &xCtx.xPmSetting);
      xCtx.bPmSettingChanged = 0;
    }

    if ( (t - xCtx.ulPmLastTime) > CFG_SENSOR_PM_POLL_RATE) {

      ret = iGp2Read (xCtx.xPmSensor);
      if (ret < 0) {

        vLog (LOG_ERR, "iGp2Read() returns %d, Unable to read PM sensor !", ret);
      }
      else {

        xCtx.ulPmLastTime = t;
        xCtx.iPmCurrent = ret;
        if (msgtype == gxPLMessageTrigger) {

          if (abs (xCtx.iPmCurrent -  xCtx.iPmLastTx) >=  xCtx.iPmGap) {

            xCtx.bPmRequest = 1;
          }
        }
      }
    }
  }

  // Envoi des messages
  gxPLMessageTypeSet (xCtx.xSensorMsg, msgtype);

  if (xCtx.bRhtUpdated) {
    if (xCtx.bTempRequest) {

      xCtx.bTempRequest = 0;
      gxPLMessageBodyClear (xCtx.xSensorMsg);
      gxPLMessagePairAdd (xCtx.xSensorMsg, "device", CFG_SENSOR_RHT_DEVICE);
      gxPLMessagePairAdd (xCtx.xSensorMsg, "type", CFG_SENSOR_TEMP_TYPE);
      gxPLMessagePairAddFormat (xCtx.xSensorMsg, "current", "%.1f", xCtx.xRhtCurrent.dTemp);

      // Broadcast the message
      PDEBUG ("sensor broadcast temp value = %.1f", xCtx.xRhtCurrent.dTemp);
      if (gxPLDeviceMessageSend (device, xCtx.xSensorMsg) < 0) {

        return -1;
      }
      xCtx.xRhtLastTx.dTemp = xCtx.xRhtCurrent.dTemp;
    }

    if (xCtx.bHumRequest) {

      xCtx.bHumRequest = 0;
      gxPLMessageBodyClear (xCtx.xSensorMsg);
      gxPLMessagePairAdd (xCtx.xSensorMsg, "device", CFG_SENSOR_RHT_DEVICE);
      gxPLMessagePairAdd (xCtx.xSensorMsg, "type", CFG_SENSOR_HUM_TYPE);
      gxPLMessagePairAddFormat (xCtx.xSensorMsg, "current", "%.1f", xCtx.xRhtCurrent.dHum);

      // Broadcast the message
      PDEBUG ("sensor broadcast humidity value = %.1f", xCtx.xRhtCurrent.dHum);
      if (gxPLDeviceMessageSend (device, xCtx.xSensorMsg) < 0) {

        return -1;
      }
      xCtx.xRhtLastTx.dHum = xCtx.xRhtCurrent.dHum;
    }
  }

  if (xCtx.bCo2Request) {

    xCtx.bCo2Request = 0;
    gxPLMessageBodyClear (xCtx.xSensorMsg);
    gxPLMessagePairAdd (xCtx.xSensorMsg, "device", CFG_SENSOR_IAQ_DEVICE);
    gxPLMessagePairAdd (xCtx.xSensorMsg, "type", CFG_SENSOR_CO2_TYPE);
    gxPLMessagePairAddFormat (xCtx.xSensorMsg, "current", "%u", xCtx.xIaqCurrent.usCo2);
    gxPLMessagePairAdd (xCtx.xSensorMsg, "units", CFG_SENSOR_CO2_UNIT);

    // Broadcast the message
    PDEBUG ("sensor broadcast co2 value = %u " CFG_SENSOR_CO2_UNIT, xCtx.xIaqCurrent.usCo2);
    if (gxPLDeviceMessageSend (device, xCtx.xSensorMsg) < 0) {

      return -1;
    }
    xCtx.xIaqLastTx.usCo2 = xCtx.xIaqCurrent.usCo2;
  }

  if (xCtx.bTvocRequest) {

    xCtx.bTvocRequest = 0;
    gxPLMessageBodyClear (xCtx.xSensorMsg);
    gxPLMessagePairAdd (xCtx.xSensorMsg, "device", CFG_SENSOR_IAQ_DEVICE);
    gxPLMessagePairAdd (xCtx.xSensorMsg, "type", CFG_SENSOR_TVOC_TYPE);
    gxPLMessagePairAddFormat (xCtx.xSensorMsg, "current", "%u", xCtx.xIaqCurrent.usTvoc);
    gxPLMessagePairAdd (xCtx.xSensorMsg, "units", CFG_SENSOR_TVOC_UNIT);

    // Broadcast the message
    PDEBUG ("sensor broadcast tvoc value = %u "CFG_SENSOR_TVOC_UNIT, xCtx.xIaqCurrent.usTvoc);
    if (gxPLDeviceMessageSend (device, xCtx.xSensorMsg) < 0) {

      return -1;
    }
    xCtx.xIaqLastTx.usTvoc = xCtx.xIaqCurrent.usTvoc;
  }

  if (xCtx.bPmRequest) {

    xCtx.bPmRequest = 0;
    gxPLMessageBodyClear (xCtx.xSensorMsg);
    gxPLMessagePairAdd (xCtx.xSensorMsg, "device", CFG_SENSOR_PM_DEVICE);
    gxPLMessagePairAdd (xCtx.xSensorMsg, "type", CFG_SENSOR_PM_TYPE);
    gxPLMessagePairAddFormat (xCtx.xSensorMsg, "current", "%u", xCtx.iPmCurrent);
    gxPLMessagePairAdd (xCtx.xSensorMsg, "units", CFG_SENSOR_PM_UNIT);

    // Broadcast the message
    PDEBUG ("sensor broadcast pm10 value = %u", xCtx.iPmCurrent);
    if (gxPLDeviceMessageSend (device, xCtx.xSensorMsg) < 0) {

      return -1;
    }
    xCtx.iPmLastTx = xCtx.iPmCurrent;
  }

  return 0;
}

// -----------------------------------------------------------------------------
static void
prvSensorMessageListener (gxPLDevice * device, gxPLMessage * msg, void * udata) {

  if (gxPLMessagePairExist (msg, "request") == true) {
    // the request key is present in the message

    if (strcmp (gxPLMessagePairGet (msg, "request"), "current") == 0) {

      // this is a request for the current value
      if (gxPLMessagePairExist (msg, "type") == true) {

        // Un seul type de mesure est demandé
        if (strcmp (gxPLMessagePairGet (msg, "type"), CFG_SENSOR_TEMP_TYPE) == 0) {

          xCtx.bTempRequest = (xCtx.xRhtSensor != NULL);
        }
        else if (strcmp (gxPLMessagePairGet (msg, "type"), CFG_SENSOR_HUM_TYPE) == 0) {

          xCtx.bHumRequest = (xCtx.xRhtSensor != NULL);
        }
        else if (strcmp (gxPLMessagePairGet (msg, "type"), CFG_SENSOR_CO2_TYPE) == 0) {

          xCtx.bCo2Request = (xCtx.xIaqSensor != NULL);
        }
        else if (strcmp (gxPLMessagePairGet (msg, "type"), CFG_SENSOR_TVOC_TYPE) == 0) {

          xCtx.bTvocRequest = (xCtx.xIaqSensor != NULL);
        }
        else if (strcmp (gxPLMessagePairGet (msg, "type"), CFG_SENSOR_PM_TYPE) == 0) {

          xCtx.bPmRequest = (xCtx.xPmSensor != NULL);
        }
      }
      else if (gxPLMessagePairExist (msg, "device") == true) {

        // Un seul capteur est demandé
        if (strcmp (gxPLMessagePairGet (msg, "device"), CFG_SENSOR_RHT_DEVICE) == 0) {

          xCtx.bTempRequest = (xCtx.xRhtSensor != NULL);
          xCtx.bHumRequest = (xCtx.xRhtSensor != NULL);
        }
        else if (strcmp (gxPLMessagePairGet (msg, "device"), CFG_SENSOR_IAQ_DEVICE) == 0) {

          xCtx.bCo2Request = (xCtx.xIaqSensor != NULL);
          xCtx.bTvocRequest = (xCtx.xIaqSensor != NULL);
        }
        else if (strcmp (gxPLMessagePairGet (msg, "device"), CFG_SENSOR_PM_DEVICE) == 0) {

          xCtx.bPmRequest = (xCtx.xPmSensor != NULL);
        }
      }
      else {

        // Toutes les mesures sont demandées
        xCtx.bTempRequest = (xCtx.xRhtSensor != NULL);
        xCtx.bHumRequest = (xCtx.xRhtSensor != NULL);
        xCtx.bCo2Request = (xCtx.xIaqSensor != NULL);
        xCtx.bTvocRequest = (xCtx.xIaqSensor != NULL);
        xCtx.bPmRequest = (xCtx.xPmSensor != NULL);

        xCtx.ulStatLastTime = time (NULL);
      }

      priSendCurrentValue (device, gxPLMessageStatus);
    }
  }
}

/* internal public functions ================================================ */
// -----------------------------------------------------------------------------
int
iSensorOpen (gxPLDevice * device) {
  bool bSensorProvided = false;

  // Setting up hardware for the sensor
  if (xCtx.ucRhtAddr) {

    xCtx.xRhtSensor = xHih6130Open (xCtx.sI2cBus, xCtx.ucRhtAddr);
    if (xCtx.xRhtSensor == NULL) {

      vLog (LOG_ERR, "Unable to open RHT sensor");
      return -1;
    }
    bSensorProvided = true;
  }

  if (xCtx.ucIaqAddr) {

    xCtx.xIaqSensor = xIaqOpen (xCtx.sI2cBus, xCtx.ucIaqAddr);
    if (xCtx.xIaqSensor == NULL) {

      vLog (LOG_ERR, "Unable to open IAQ sensor");
      iHih6130Close (xCtx.xRhtSensor);
      return -1;
    }
    bSensorProvided = true;
  }

  if (xCtx.ucPmAddr) {

    xCtx.xPmSensor = xGp2Open (xCtx.sI2cBus, xCtx.ucPmAddr, &xCtx.xPmSetting);
    if (xCtx.xPmSensor == NULL) {

      vLog (LOG_ERR, "Unable to open PM sensor");
      iHih6130Close (xCtx.xRhtSensor);
      iIaqClose (xCtx.xIaqSensor);
      return -1;
    }
    bSensorProvided = true;
  }

  if (bSensorProvided) {
    // Add a responder for sensor.request schema
    return gxPLDeviceListenerAdd (device, prvSensorMessageListener,
                                  gxPLMessageCommand, "sensor", "request", NULL);
  }

  vLog (LOG_ERR, "No sensor specified");
  return -1;
}

// -----------------------------------------------------------------------------
int
iSensorClose (gxPLDevice * device) {
  int ret = 0;

  // Add here the necessary steps to close the sensor
  if (xCtx.xRhtSensor) {
    ret = iHih6130Close (xCtx.xRhtSensor);
  }

  if (xCtx.xIaqSensor) {
    ret += iIaqClose (xCtx.xIaqSensor);
  }

  if (xCtx.xPmSensor) {
    ret += iGp2Close (xCtx.xPmSensor);
  }
  gxPLMessageDelete (xCtx.xSensorMsg);
  return ret;
}

// -----------------------------------------------------------------------------
int
iSensorPoll (gxPLDevice * device) {

  if (xCtx.ulStatInterval) {
    // Envoi des messages stat périodiques

    time_t t = time (NULL);

    if ( (t - xCtx.ulStatLastTime) > xCtx.ulStatInterval) {

      if (xCtx.xRhtSensor != NULL) {

        if  (xCtx.bRhtUpdated == 0) {
          int ret = -1;

          priRhtStart();

          // Attente première mesure RHT
          while (ret != 0)  {

            ret = priRhtRead (&xCtx.xRhtCurrent);
            if (ret < 0) {

              return ret;
            }
          }
          xCtx.bRhtUpdated = 1;
        }

        xCtx.bTempRequest = 1;
        xCtx.bHumRequest = 1;
      }

      // Toutes les mesures seront envoyées
      xCtx.bCo2Request = (xCtx.xIaqSensor != NULL);
      xCtx.bTvocRequest = (xCtx.xIaqSensor != NULL);
      xCtx.bPmRequest = (xCtx.xPmSensor != NULL);

      xCtx.ulStatLastTime = t;
      return priSendCurrentValue (device, gxPLMessageStatus);
    }
  }

  return priSendCurrentValue (device, gxPLMessageTrigger);
}

/* ========================================================================== */
