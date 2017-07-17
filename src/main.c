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
 * @brief Surveillance de qualité de l'air - programme principal
 */
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <sysio/string.h>
#include "dmg-iaqmon.h"
#include "config.h"

/* public variables ========================================================= */
xIaqMonContext xCtx;

/* private variables ======================================================== */
static bool bMainIsRun = true;

/* private functions ======================================================== */
// -----------------------------------------------------------------------------
// Print usage info
static void
prvPrintUsage (void) {
  printf ("%s - xPL Indoor Air Quality Monitor\n", __progname);
  printf ("Copyright (c) 2016-2017 epsilonRT\n\n");
  printf ("Usage: %s [-i interface] [-n iolayer] [-W timeout] [-b i2cbus] [-q [address]] [-t [address]] [-D] [-d] [-h]\n", __progname);
  printf ("  -i interface - use interface named interface (i.e. eth0) as network interface\n");
  printf ("  -n iolayer   - use hardware abstraction layer to access the network\n"
          "                 (i.e. udp, xbeezb... default: udp)\n");
  printf ("  -W timeout   - set the timeout at the opening of the io layer\n");
  printf ("  -D           - do not daemonize -- run from the console\n");
  printf ("  -d           - enable debugging, it can be doubled or tripled to\n"
          "                 increase the level of debug.\n");
  printf ("  -b i2cbus    - i2c bus used by the sensors.\n"
          "                 default: %s\n", CFG_DEFAULT_I2C_BUS);
  printf ("  -q [address] - enable IAQ Core P sensor, the address on the bus can be\n"
          "                 supplied, otherwise default is 0x%02X.\n", CFG_DEFAULT_IAQ_ADDR);
  printf ("  -t [address] - enable ChipCap2/Hih sensor, the address on the bus can be\n"
          "                 supplied, otherwise default is 0x%02X.\n", CFG_DEFAULT_RHT_ADDR);
  printf ("  -h           - print this message\n\n");
}

// -----------------------------------------------------------------------------
static void
prvSignalHandler (int sig) {

  if ( (sig == SIGTERM) || (sig == SIGINT) ) {

    bMainIsRun = false;
  }
}

/* internal public functions ================================================ */
// -----------------------------------------------------------------------------
void
vMain (gxPLSetting * setting) {
  int ret;
  gxPLDevice * device;
  gxPLApplication * app;

  PNOTICE ("starting epsirt-iaqmon... (%s log)", sLogPriorityStr (setting->log) );

  // Create xPL application and device
  device = xDeviceCreate (setting);
  if (device == NULL) {

    vLog (LOG_ERR, "Unable to start xPL");
    exit (EXIT_FAILURE);
  }

  // take the application to be able to close
  app = gxPLDeviceParent (device);

  // Sensor init.
  ret = iSensorOpen (device);
  if (ret != 0) {

    vLog (LOG_ERR, "Unable to setting up sensor, error %d", ret);
    gxPLAppClose (app);
    exit (EXIT_FAILURE);
  }

  // Enable the device to do the job
  gxPLDeviceEnable (device, true);

  // Install signal traps for proper shutdown
  signal (SIGTERM, prvSignalHandler);
  signal (SIGINT, prvSignalHandler);

  if (setting->nodaemon == 0) {
    
    vLogDaemonize (true);
  }
  
  while (bMainIsRun) {

    // Main Loop
    ret = gxPLAppPoll (app, CFG_XPL_POLL_RATE_MS);
    if (ret != 0) {

      PWARNING ("Unable to poll xPL network, error %d", ret);
    }

    if (gxPLDeviceIsHubConfirmed (device) ) {

      // if the hub is confirmed, performs xPL tasks...
      ret = iSensorPoll (device);
      if (ret != 0) {

        PWARNING ("Unable to poll sensor, error %d", ret);
      }
    }
  }

  ret = iSensorClose (device);
  if (ret != 0) {

    PWARNING ("Unable to close sensor, error %d", ret);
  }

  // Sends heartbeat end messages to all devices
  ret = gxPLAppClose (app);
  if (ret != 0) {

    PWARNING ("Unable to close xPL network, error %d", ret);
  }

  PNOTICE ("dmg-iaqmon closed, Have a nice day !");
  exit (EXIT_SUCCESS);
}


// -----------------------------------------------------------------------------
void
vParseAdditionnalOptions (int argc, char *argv[]) {
  int c;

  static const char short_options[] = "hb:q::t::p::" GXPL_GETOPT;
  static struct option long_options[] = {
    {"bus",     required_argument, NULL, 's'},
    {"iaq",     optional_argument, NULL, 'q' },
    {"rht",     optional_argument, NULL, 't' },
    {"pm",     optional_argument, NULL, 'p' },
    {"help",     no_argument,       NULL, 'h' },
    {NULL, 0, NULL, 0} /* End of array need by getopt_long do not delete it*/
  };

  xCtx.sI2cBus = CFG_DEFAULT_I2C_BUS;

  do  {

    c = getopt_long (argc, argv, short_options, long_options, NULL);

    switch (c) {

      case 'b':
        xCtx.sI2cBus = optarg;
        PDEBUG ("set i2c bus to %s", xCtx.sI2cBus);
        break;

      case 'q':
        xCtx.ucIaqAddr = CFG_DEFAULT_IAQ_ADDR;
        if (optarg) {
          long n;

          if (iStrToLong (optarg, &n, 0) == 0) {
            if ( (n >= 0x03) && (n <= 0x77) ) {

              xCtx.ucIaqAddr = (unsigned) n;
              PDEBUG ("set IAQ Core P address to 0x%02X", xCtx.ucIaqAddr);
              break;
            }
          }
          PERROR ("Unable to set IAQ Core P address to %s", optarg);
          break;
        }
        PDEBUG ("enabled IAQ Core P at 0x%02X (default)", xCtx.ucIaqAddr);
        break;

      case 't':
        xCtx.ucRhtAddr = CFG_DEFAULT_RHT_ADDR;
        if (optarg) {
          long n;

          if (iStrToLong (optarg, &n, 0) == 0) {
            if ( (n >= 0x03) && (n <= 0x77) ) {

              xCtx.ucRhtAddr = (unsigned) n;
              PDEBUG ("set ChipCap2/Hih address to 0x%02X", xCtx.ucRhtAddr);
              break;
            }
          }
          PERROR ("Unable to set ChipCap2/Hih address to %s", optarg);
          break;
        }
        PDEBUG ("enabled ChipCap2/Hih at 0x%02X (default)", xCtx.ucRhtAddr);
        break;

      case 'p':
        xCtx.ucPmAddr = CFG_DEFAULT_PM_ADDR;
        if (optarg) {
          long n;

          if (iStrToLong (optarg, &n, 0) == 0) {
            if ( (n >= 0x03) && (n <= 0x77) ) {

              xCtx.ucPmAddr = (unsigned) n;
              PDEBUG ("set gp2-i2c address to 0x%02X", xCtx.ucPmAddr);
              break;
            }
          }
          PERROR ("Unable to set gp2-i2c address to %s", optarg);
          break;
        }
        PDEBUG ("enabled gp2-i2c at 0x%02X (default)", xCtx.ucRhtAddr);
        break;

      case 'h':
        prvPrintUsage();
        exit (EXIT_SUCCESS);
        break;

      default:
        break;
    }
  }
  while (c != -1);
}

/* ========================================================================== */
