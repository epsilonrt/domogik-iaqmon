# dmg-iaqmon

*Moniteur de Qualité de l'air intérieur*

---
Copyright 2016-2017 (c), epsilonRT

<a href="http://www.cecill.info/licences/Licence_CeCILL_V2.1-en.html">
  <img src="https://raw.githubusercontent.com/epsilonrt/gxPL/master/doc/images/osi.png" alt="osi.png" align="right" valign="top">
</a>

dmg-iaqmon est un daemon qui surveille la qualité de l'air intérieur. Il utilise 
pour cela 3 capteurs sur bus I²C:

* Un capteur d'humidité et de température [ChipCap®2](http://amphenol-sensors.com/en/products/humidity/relative-humidity-sensors/3095-chipcap2-humidity-and-temperature-sensor-system-on-a-chip)  
* Un capteur de CO² et de composés organiques volatiles [IAQ-Core P](http://ams.com/eng/Products/Environmental-Sensors/Air-Quality-Sensors/iAQ-core-P)  
* Un capteur de particules fines [GP2Y1010](https://www.sharpsde.com/products/optoelectronic-components/model/GP2Y1010AU0F/) équipé d'un module [gp2-i2c](https://github.com/epsilonrt/gp2-i2c)

Au moins un des capteurs doit être connecté au bus I²C.

Il a été développé sur un [Raspberry Pi](https://www.raspberrypi.org/) 
mais peut être installé sur n'importe quelle cible suportée par SysIo et gxPL.

## Installation

### Pré-requis

Il faut installer au prélable [SysIo](https://github.com/epsilonrt/sysio) et 
[gxPL](https://github.com/epsilonrt/gxPL).

### Compilation et Installation
 
Il suffit alors d'exécuter les commandes suivantes :

        git clone https://github.com/epsilonrt/domogik-iaqmon.git
        cd domogik-iaqmon
        make
        sudo make install

### Installation script de démarrage

Pour automatiser le démarrage et l'arrêt, il est possible d'installer le script de démarrage:

        cd init
        sudo make install

## Commande

    dmg-iaqmon - xPL Indoor Air Quality Monitor
    Copyright (c) 2016-2017 epsilonRT

    Usage: dmg-iaqmon [-i interface] [-n iolayer] [-W timeout] [-b i2cbus] [-q [address]] [-t [address]] [-D] [-d] [-h]
      -i interface - use interface named interface (i.e. eth0) as network interface
      -n iolayer   - use hardware abstraction layer to access the network
                     (i.e. udp, xbeezb... default: udp)
      -W timeout   - set the timeout at the opening of the io layer
      -D           - do not daemonize -- run from the console
      -d           - enable debugging, it can be doubled or tripled to
                     increase the level of debug.
      -b i2cbus    - i2c bus used by the sensors.
                     default: /dev/i2c-1
      -q [address] - enable IAQ Core P sensor, the address on the bus can be
                     supplied, otherwise default is 0x5A.
      -t [address] - enable ChipCap2/Hih sensor, the address on the bus can be
                     supplied, otherwise default is 0x28.
      -h           - print this message

Pour lancer le daemon en mode débugage avec gestion d'un capteur IAQ Core P et 
d'un capteur ChipCap®2:

    dmg-iaqmon -q -t -D -ddd

Un script de lancement dmg-iaqmon peut être installé dans /etc/init.d et peut être
lancé à l'aide de la commande:

    sudo /etc/init.d/dmg-iaqmon start

Il peut être lancé automatiquement au démarrage du système:

    sudo insserv dmg-iaqmon

## Messages xPL

**dmg-iaqmon** est un __device__ xPL configurable, son identification par défaut sur
xPL est: **domogik-iaqmon**.instanceid

L'id d'instance est aléatoire et unique au premier démarrage et peut être 
personnalisé grâce au protocole de configuration prévu par xPL (paramètre **newconf**).

**dmg-iaqmon** utilise le schéma **sensor.basic** pour transmettre ses mesures. 

Le champ **device** peut prendre 3 valeurs en fonction du capteurs fournissant 
la mesure:

1. **rht** pour le capteur ChipCap®2 de température et d'humidité  
2. **iaq** pour le capteur IAQ-Core P de CO² et de composés organiques volatiles  
3. **pm** pour le capteur GP2Y1010 de particules fines.

**dmg-iaqmon** peut être interrogé grâce à un message **sensor.request**.

### Messages **xpl-stat**

Ces messages sont émis à un intervale correspondant au paramètre configurable
**stat-interval** .

#### Température

    sensor.basic
    {
    device=rht
    type=temp
    current=<value> # Valeur de la température en °C (float)
    }

#### Humidité

    sensor.basic
    {
    device=rht
    type=humidity
    current=<value> # Valeur de l'humité en %RH (float)
    }

#### Taux de CO²

    sensor.basic
    {
    device=iaq
    type=co2
    current=<value> # Valeur du taux de CO² (unsigned int)
    units=ppm
    }

#### Taux de composés organiques volatiles

    sensor.basic
    {
    device=iaq
    type=tvoc
    current=<value> # Valeur du taux de VOC (unsigned int)
    units=ppb
    }

#### Particules fines

    sensor.basic
    {
    device=pm
    type=pm10
    current=<value> # Densité de particules fines en µg/m3 (float)
    units=ug_m3
    }

### Messages **xpl-trig**

Ces messages sont émis lorsqu'une valeur évolue d'un écart supérieur ou égal 
au gap réglé par la configuration xPL.

Le corps des messages est le même que que les messages XPL-STAT.

### Messages **xpl-cmnd**

    sensor.request
    {
    request=current
    [device=<sensor_name>] # Nom du capteur: rht, iaq ou pm
    [type=<value_name>] # Valeur à mesurer: temp ou humidity pour rht, co2 ou tvoc pour iaq, pm10 pour pm
    }

Ce message permet de demander la mesure d'une ou plusieurs valeurs. Si le message
est correctement formaté, un message **xpl-stat** du schéma **sensor.basic** 
est envoyé en réponse.

Plusieurs cas sont pris en charge:

* Champs **device** et **type** non présents: toutes les mesures sont renvoyées
* Champs **device** présent: toutes les mesures du capteur correspondant sont renvoyées
* Champs **device** et **type** présents: seule la mesure correspondante est renvoyée.

## Paramètres configurables

Les paramètres ci-dessous sont configurables par l'intermédiaire du protocole
de configuration prévu par xPL :

* **stat-interval** délai en secondes entre 2 envois de trame XPL-STAT, sa
valeur par défaut est de 300 s, une valeur de 0 vzut dire qu'aucun message
XPL-STAT ne sera transmis (uniquement des messages XPL-TRIG si l'écart entre
2 mesures dépasse le gap).  
* **temp-gap** écart de température nécessaire à la transmission d'un message xpl-trig  
* **temp-zero** correction de zéro de la température
* **hum-gap** écart d'humidité nécessaire à la transmission d'un message xpl-trig  
* **hum-zero** correction de zéro de l'humidité
* **co2-gap** écart de co2 nécessaire à la transmission d'un message xpl-trig  
* **tvoc-gap** écart de tvoc nécessaire à la transmission d'un message xpl-trig  
* **pm10-gap** écart de PM10 nécessaire à la transmission d'un message xpl-trig  
* **pm10-d1** valeur de densité de PM10 en µg/m3 correspondant au premier point d'étalonnage (densité la plus faible)  
* **pm10-v1** valeur de tension en mV correspondant au premier point d'étalonnage  
* **pm10-d2** valeur de densité de PM10 en µg/m3 correspondant au deuxième point d'étalonnage (densité la plus forte)  
* **pm10-v2** valeur de tension en mV correspondant au deuxième point d'étalonnage  
* **interval** interval en minutes entre 2 battements de coeur  
* **newconf** nom de la configuration (instance)  
* **group** groupes dont dmg-iaqmon fait partie, voir 
  [Devices Groups](http://xplproject.org.uk/wiki/XPL_Specification_Document.html#Device_Groups)  
* **filter** filtre de messages utilisés, voir 
  [Installer and User Defined Filters](http://xplproject.org.uk/wiki/XPL_Specification_Document.html#Installer_and_User_Defined_Filters)
