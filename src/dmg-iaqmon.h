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

  time_t ulStatLastTime;

  gxPLMessage * xSensorMsg;
  struct {
    int bRhtStarted: 1; // Mesure démarrée
    int bRhtUpdated: 1; // 1ère mesure effectuée
    
    int bTempRequest: 1;
    int bHumRequest: 1;
    int bCo2Request: 1;
    int bTvocRequest: 1;
    int bPmRequest: 1;
    int bPmSettingChanged: 1;
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

/* ========================================================================== */
#endif /* _DMG_IAQMON_HEADER_ defined */
