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
#include <sysio/ledrgb.h>
#include <sysio/log.h>
#include <sysio/string.h>
#include "dmg-iaqmon.h"
#include "config.h"

/* private variables ======================================================== */
static xLedRgbDevice * dev;
// liste des couleurs utilisées
static uint32_t aqi2color[6] = {RGB_BLUE, RGB_GREEN, RGB_YELLOW, RGB_ORANGE, RGB_PURPLE, RGB_RED};

/* private functions ======================================================== */

// -----------------------------------------------------------------------------
// Modification luminosité led
static void
prvControlBasicListener (gxPLDevice * device, gxPLMessage * msg, void * udata) {
  /*  --- Message reçu ---
        XPL-CMND Structure
        control.basic
        {
        device=led
        type=luminosity
        current=<value to which device should be set>
        }
      --- Réponse ---
        un message trig sensor.basic sera envoyé si changement d'état
   */
  if (gxPLMessagePairExist (msg, "device") &&
      gxPLMessagePairExist (msg, "type") &&
      gxPLMessagePairExist (msg, "current") ) {
    const char * dev = gxPLMessagePairGet (msg, "device");
    const char * type = gxPLMessagePairGet (msg, "type");


    if ( (strcmp (dev, CFG_SENSOR_LED_DEVICE) == 0) &&
         (strcmp (type, CFG_SENSOR_LUM_TYPE) == 0) ) {
      long n;
      const char * current = gxPLMessagePairGet (msg, "current");

      if (iStrToLong (current, &n, 0) == 0) {

        if ( (n >= 0) && (n <= 1023) ) {

          uint16_t usLedLum = (uint16_t) n;

          if (xCtx.usLedLum != usLedLum) {

            if (iLedSetLuminosity (usLedLum) == 0) {

              xCtx.usLedLum = usLedLum;
              gxPLMessageTypeSet (xCtx.xSensorMsg, gxPLMessageTrigger);
              gxPLMessageBodyClear (xCtx.xSensorMsg);
              gxPLMessagePairAdd (xCtx.xSensorMsg, "device", CFG_SENSOR_LED_DEVICE);
              gxPLMessagePairAdd (xCtx.xSensorMsg, "type", CFG_SENSOR_LUM_TYPE);
              gxPLMessagePairAddFormat (xCtx.xSensorMsg, "current", "%u", xCtx.usLedLum);

              // Broadcast the message
              PDEBUG ("set led luminosity = %u", xCtx.usLedLum);
              gxPLDeviceMessageSend (device, xCtx.xSensorMsg);
            }
          }
        }
      }
    }
  }
}

/* internal public functions ================================================ */

// -----------------------------------------------------------------------------
int
iLedOpen (gxPLDevice * device) {
  int ret;
  /*
   * Liste des TLC59116 présents
   * Chaque circuit TLC59116 est défini par le bus I²C sur lequel il se trouve
   * (i2c_bus) et par son adresse I²C (c.f. § 10.1.2 du datasheet p. 23).
   * La liste des TLC59116 doit se terminer avec un élément dont le champs
   * i2c_bus vaut NULL.
   * Ici nous avons un seul contrôleur TLC59116 connecté sur le bus i2c-1 à
   * l'adresse TLC59116_ADDR (0, 0, 0, 0), c'est à dire, 110 0000 (0x60 donc)
   */
  xTlc59116Config tlc59116_list[] = {
    {.i2c_bus = CFG_DEFAULT_I2C_BUS, .i2c_addr = CFG_DEFAULT_LED_ADDR },
    /* Fin de la liste */ {.i2c_bus = NULL, 0}
  };
  /*
   * Liste des leds RGB
   * Chaque led est définie par 3 broches : red, green, blue.
   * Chaque broche est définie par son numéro de sortie (0 à 15 pour OUT0 à OUT15)
   * et par son numéro de contrôleur TLC59116.
   * Ici nous avons 4 leds, toutes connectées au seul contrôleur présent (0 donc).
   */
  xTlc59116Led setup[CFG_DEFAULT_LED_NOF_LEDS] = {
    {
      .red = { .out = CFG_DEFAULT_LED_LED1_RED, .ctrl = 0 },
      .green = { .out =  CFG_DEFAULT_LED_LED1_GREEN, .ctrl = 0 },
      .blue = { .out =  CFG_DEFAULT_LED_LED1_BLUE, .ctrl = 0 }
    },
    {
      .red = { .out = CFG_DEFAULT_LED_LED2_RED, .ctrl = 0 },
      .green = { .out = CFG_DEFAULT_LED_LED2_GREEN, .ctrl = 0 },
      .blue = { .out = CFG_DEFAULT_LED_LED2_BLUE, .ctrl = 0 }
    },
    {
      .red = { .out = CFG_DEFAULT_LED_LED3_RED, .ctrl = 0 },
      .green = { .out =  CFG_DEFAULT_LED_LED3_GREEN, .ctrl = 0 },
      .blue = { .out =  CFG_DEFAULT_LED_LED3_BLUE, .ctrl = 0 }
    },
    {
      .red = { .out = CFG_DEFAULT_LED_LED4_RED, .ctrl = 0 },
      .green = { .out =  CFG_DEFAULT_LED_LED4_GREEN, .ctrl = 0 },
      .blue = { .out =  CFG_DEFAULT_LED_LED4_BLUE, .ctrl = 0 }
    }
  };

  /*
   * Création et initialisation de l'objet permettant le contrôle des leds
   * Un pointeur sur l'objet créé est renvoyé, NULL si erreur.
   * La présence des contrôleurs TLC59116 est vérifiée et il sont initialisés
   */
  dev = xLedRgbNewDevice (eLedRgbDeviceTlc59116, tlc59116_list);
  if (dev == NULL) {

    vLog (LOG_ERR, "Unable to connect to TLC59116 controllers, check their configuration or hardware !\n");
    return -1;
  }

  /*
   * Ajout des leds
   * Les leds sont configurés en mode complet (couleur, luminosité, clignotement)
   */
  for (int i = 0; i < CFG_DEFAULT_LED_NOF_LEDS; i++) {

    ret = iLedRgbAddLed (dev, eLedRgbModeFull, &setup[i]);
    if (ret < 0) {

      vLog (LOG_ERR, "Unable to add led number %d !\n", i);
      iLedRgbDeleteDevice (dev);
      return -1;
    }
  }

  gxPLDeviceListenerAdd (device, prvControlBasicListener,
                         gxPLMessageCommand, "control", "basic", NULL);
  xCtx.usLedLum = CFG_DEFAULT_LED_LUM;
  return iLedSetLuminosity (xCtx.usLedLum);
}

// -----------------------------------------------------------------------------
int
iLedClose (void) {

  // Extinction de toutes les leds
  iLedRgbSetGrpMode (dev, LEDRGB_ALL_LEDS, eLedRgbModeOff);

  // Destruction de l'objet LedRgb
  return iLedRgbDeleteDevice (dev);
}

// -----------------------------------------------------------------------------
int
iLedSetColor (uint8_t aqi) {

  if ( (aqi >= 1) && (aqi <= 6) ) {

    return  iLedRgbSetGrpColor (dev, LEDRGB_ALL_LEDS, aqi2color[aqi - 1]);
  }
  return -1;
}

// -----------------------------------------------------------------------------
int
iLedSetLuminosity (uint16_t lum) {

  return iLedRgbSetDimmer (dev, 0, lum >> 2);
}

/* ========================================================================== */