# mqtt-esp8266-switch
Sonoff ESP8266 mqtt switch

This code goes with an article I wrote describing using a Sonoff Basic WiFi switch with MQTT. A Raspberry Pi is setup with Mosquitto MQTT. The switch is programmed using Arduino IDE and the ESP8266 generic community board configuration.

For details see [Sonoff IoT Switch using Mosquitto MQTT and Arduino ESP8266](http://tmblog.mwbinc.com/general/2018/02/11/sonoff-IoT-switch-using-MQTT-ESP8266.html)

**Features**

* LED light is on when the switch is on
* Switch button controls the light
* Turn on or off from MQTT commands
* Sends status after connecting to MQTT
* Send status when switch pressed

I used the following settings in the Arduino IDE for the Sonoff switch ESP8266.

| Setting | Value |
|------|---------|
| Board   | Generic ESP8266 Module |
| Upload Speed |  115200 |
| CPU Frequency |  80 MHz |
| Flash Mode |  DOUT |
| Flash Size |  1M (no SPIFFS) |
| CPU Frequency | 80 MHz |
| Crystal Frequency | 26 MHz |
| Reset Method |  ck |
| Debug port | Disabled |
| Debug Level | None |
| IwIP Variant | v2 Lower Memory |
| VTables | Flash |
| Builtin Led |  13 |
| Port |  the port specified with USB serial connection |

