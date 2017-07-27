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
 * @brief Surveillance de qualité de l'air - header
 */
#ifndef _DMG_IAQMON_HEADER_
#define _DMG_IAQMON_HEADER_
#include <stdlib.h>
#include <time.h>
#include <sysio/hih6130.h>
#include <sysio/iaq.h>
#include <sysio/gp2.h>
#include <gxPL.h>
#include <gxPL/stdio.h>
#include "version-git.h"

/* structures =============================================================== */
union xQiList {
  struct {
    uint8_t ucAqi;
    uint8_t ucCo2;
    uint8_t ucVoc;
    uint8_t ucPm;
    uint8_t ucHum;
  };
  uint8_t ucRaw[5];
};
typedef union xQiList xQiList;

struct xIaqMonContext {
  time_t ulStatInterval;  // configurable

  char * sI2cBus;

  uint8_t ucRhtAddr;
  xHih6130 * xRhtSensor;
  xHih6130Data xRhtCurrent;
  xHih6130Data xRhtLastTx;
  xHih6130Data xRhtGap;   // configurable
  xHih6130Data xRhtZero;  // configurable

  uint8_t ucIaqAddr;
  xIaq * xIaqSensor;
  xIaqData xIaqCurrent;
  xIaqData xIaqLastTx;
  xIaqData xIaqGap;       // configurable
  time_t ulIaqLastTime;

  uint8_t ucPmAddr;
  xG2pSensor * xPmSensor;
  int iPmCurrent;
  int iPmLastTx;
  int iPmGap;          // configurable
  xG2pSetting xPmSetting; // configurable
  time_t ulPmLastTime;

  xQiList xQiCurrent;
  xQiList xQiLastTx;

  uint16_t usLedMaxToForce; // from command line
  uint16_t usLedMax; // configurable 0-1023
  uint8_t ucLedSlider; // 0-255

  time_t ulStatLastTime;

  gxPLMessage * xSensorMsg;
  uint8_t ucFlag;
#define IAQF_CO2 1
#define IAQF_VOC 2
#define IAQF_PM  4
#define IAQF_HUM 8
  struct {
    uint16_t bRhtStarted: 1; // Mesure démarrée
    uint16_t bRhtUpdated: 1; // 1ère mesure effectuée

    uint16_t bAqiEnabled: 1;
    uint16_t bAqiRequest: 1;

    uint16_t bTempRequest: 1;

    uint16_t bHumRequest: 1;
    uint16_t bHumQiRequest: 1;

    uint16_t bCo2Request: 1;
    uint16_t bCo2QiRequest: 1;

    uint16_t bTvocRequest: 1;
    uint16_t bTvocQiRequest: 1;

    uint16_t bPmRequest: 1;
    uint16_t bPmQiRequest: 1;
    uint16_t bPmSettingChanged: 1;

    uint16_t bLedEnabled: 1;
    uint16_t bLedRequest: 1;
    uint16_t bLedChanged: 1;
  };
};
typedef struct xIaqMonContext xIaqMonContext;

/* public variables ========================================================= */
/*
 * Configurable parameters
 */
extern xIaqMonContext xCtx;

/* internal public functions ================================================ */
/*
 * Main Task
 */
void vMain (gxPLSetting * setting);
void vParseAdditionnalOptions (int argc, char ** argv);

/*
 * xPL Device
 */
gxPLDevice * xDeviceCreate (gxPLSetting * setting);

/*
 * Sensor Interface
 */
int iSensorOpen (gxPLDevice * device);
int iSensorClose (gxPLDevice * device);
int iSensorPoll (gxPLDevice * device);

/*
 * RGB Led Interface
 */
int iLedOpen (gxPLDevice * device);
int iLedClose (void);
int iLedSetColor (uint8_t aqi);
int iLedSetLuminosity (uint16_t lum);

/* ========================================================================== */
#endif /* _DMG_IAQMON_HEADER_ defined */
