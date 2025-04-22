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
  Serial.begin(115200);
  api.lorawan.nwm.set();
  api.lorawan.deui.get(lorawan_credentials.dev_eui, 8);
  api.lorawan.appeui.get(lorawan_credentials.dev_eui, 8);
  api.lorawan.appkey.get(lorawan_credentials.dev_eui, 16);
  CfgServer.enable();

}

void loop()
{
  DEBUG_SERIAL_PRINTF("DEVEUI: " DEVEUI_STR "\r\n", DEVEUI(lorawan_credentials.dev_eui));
  CfgServer.listen(&lorawan_credentials, &lorawan_params,  &edlo_timing, &edlo_datetime);
  delay(1000);
}
