#include "common.h"
#include "p2p_receiver.h"

sLoRa lorawan_credentials;
sLoRaParameters lorawan_params = {
  .dr = 3,
  .adr_enabled = 1,
  .sub_band_mask = 0x01,
  .rx1_delay = 5,
  .rx2_delay = 10,
  .retry = 2,
};

EdLoDatetime edlo_datetime = {
  .yOff = 24,
  .m = 4,
  .d = 14,
  .hh = 9,
  .mm = 57,
  .ss = 0
};

EdLoTiming edlo_timing = {
  .LastDatetimeSaved = {0},
  .period_to_send = 10,
  .delta_max_period = 6,
};

void setup()
{
  bool is_ok;
  Serial.begin(115200);

  is_ok = api.lorawan.nwm.set();
  DEBUG_SERIAL_PRINTF("LoRaWAN set %s\r\n", (is_ok == true)? "success" : "fail");

  is_ok = api.lorawan.deui.get(lorawan_credentials.dev_eui, 8);
  DEBUG_SERIAL_PRINTF("Load DEVEUI %s\r\n", (is_ok == true)? "success" : "fail");

  is_ok = api.lorawan.appeui.get(lorawan_credentials.app_eui, 8);
  DEBUG_SERIAL_PRINTF("Load APPEUI %s\r\n", (is_ok == true)? "success" : "fail");

  is_ok = api.lorawan.appkey.get(lorawan_credentials.app_key, 16);
  DEBUG_SERIAL_PRINTF("Load APPKEY %s\r\n", (is_ok == true)? "success" : "fail");

  if (is_ok == true) {
    DEBUG_SERIAL_PRINTF("DEVEUI: " DEVEUI_STR "\r\n", DEVEUI(lorawan_credentials.dev_eui));
    DEBUG_SERIAL_PRINTF("APPEUI: " DEVEUI_STR "\r\n", DEVEUI(lorawan_credentials.app_eui));
    DEBUG_SERIAL_PRINTF("APPKEY: " DEVEUI_STR ":" DEVEUI_STR "\r\n",
                        APPKEY(lorawan_credentials.app_key));
  }
  CfgServer.enable();
}

void loop()
{
  //api.lorawan.appeui.get(lorawan_credentials.app_eui, 8);
  //DEBUG_SERIAL_PRINTF("DEVEUI: " DEVEUI_STR "\r\n", DEVEUI(lorawan_credentials.dev_eui));
  //DEBUG_SERIAL_PRINTF("APPEUI: " DEVEUI_STR "\r\n", DEVEUI(lorawan_credentials.app_eui));
  CfgServer.listen(&lorawan_credentials, &lorawan_params,  &edlo_timing, &edlo_datetime);
  delay(1000);
}
