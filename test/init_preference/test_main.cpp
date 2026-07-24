#include "Top_Lvl_Config.h"
#include <unity.h>
#include <Arduino.h>
#include "Prog_Config.h"
#include "Preferences.h"

Preferences prefs;

void setUp(void)
{
  // set stuff up here
}

void tearDown(void)
{
  // clean stuff up here
}

void setup() {
    prefs.begin("myPrefs");
    prefs.putBool("NOT_FIRST_BOOT", true);
    prefs.putUChar("TX_TO_MODEM_RX", 17);
    prefs.putUChar("RX_TO_MODEM_TX", 16);
    prefs.putUChar("MODEM_DC_PIN", 15);
    prefs.putUChar("MODEM_DTR_PIN", 4);
    prefs.putString("APN", "v-internet");
    prefs.putString("GPRS_USER", "");
    prefs.putString("GPRS_PASS", "");
    prefs.putInt("NTRIP_MODE", 1);
    prefs.putString("NTRIP_SERVER", "ntrip.aitogy.com");
    prefs.putInt("NTRIP_PORT", 2101);
    prefs.putString("NTRIP_MOUNTPOINT", "/test");
    prefs.putString("NTRIP_AUTH_BASE_STATION", "12345");
    prefs.putString("MQTT_SERVER", "aitogy.asia");
    prefs.putInt("MQTT_PORT", 1883);
    prefs.putString("MQTT_USER", "mqttUser");
    prefs.putString("MQTT_PASS", "MqttPassword123$%^");
    prefs.putString("TOPIC_SUB_CMD", "tdm2402/um980_base_001/cmd");
    prefs.putString("TOPIC_PUB_RAW_RTCM", "tdm2402/um980_base_001/raw/last_rtcm");
    prefs.putString("TOPIC_PUB_HEALTH", "tdm2402/um980_base_001/health");
}

void loop() {
  // Nothing to do here, as the tests are run in `setup()`
}