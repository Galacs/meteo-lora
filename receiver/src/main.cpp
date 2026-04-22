#include <Arduino.h>
#include <RH_RF95.h>
#include <SPI.h>
#include "messages.h"

// Singleton instance of the radio driver
RH_RF95 rf95;
float frequency = 868.1;
void setup() {

  Serial.begin(115200);
  while (!Serial)
    ; // Wait for serial port to be available
  Serial.println("Start Sketch");
  if (!rf95.init())
    Serial.println("init failed");
  // Setup ISM frequency
  rf95.setFrequency(frequency);
  // Setup Power,dBm
  rf95.setTxPower(18);

  // Setup Spreading Factor
  rf95.setSpreadingFactor(8);

  // Setup BandWidth, option
  rf95.setSignalBandwidth(125000);

  // Setup Coding Rate:5(4/5),6(4/6),7(4/7),8(4/8)
  rf95.setCodingRate4(5);

  Serial.print("Listening on frequency: ");
  Serial.println(frequency);
}

void loop() {
  if (rf95.available()) {
    /*uint8_t data[] = "english or spanish ?";
    rf95.send(data, sizeof(data));
    rf95.waitPacketSent();
    Serial.println("Sent a reply");*/
    // Should be a message for us now
    // uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    weather_data_t msg;
    // memcpy(phrase, msg.buf, 6);
    uint8_t len = sizeof(msg);
    if (rf95.recv((uint8_t *)&msg, &len)) {
      // RH_RF95::printBuffer("request: ", buf, len);
      // Serial.printf("Values: %d, %d\n", msg.lastRSSI, msg.lastSNR);
      Serial.print("SI Temperature: ");
      Serial.println(msg.si7021_temperature);
      Serial.print("SI Humidité: ");
      Serial.println(msg.si7021_humidity);

      Serial.print("MPL Temperature: ");
      Serial.println(msg.mpl_temperature);
      Serial.print("MPL Pression: ");
      Serial.println(msg.mpl_pressure);
      Serial.print("Wind direction: ");
      Serial.println(msg.wind_direction);
      Serial.print("Wind speed: ");
      Serial.println(msg.wind_speed);
      Serial.print((" with an SNR of: "));

      Serial.print(rf95.lastSNR());
      Serial.print((" an RRSI of: "));
      Serial.println(rf95.lastRssi());
    }

    // Send a reply
    // uint8_t data[] = "english or spanish ?";
    // rf95.send((uint8_t *)&msg, sizeof(msg));
    // Serial.println(("sent"));
    // rf95.waitPacketSent();
    // Serial.println("Sent a reply");

    // } else {
    //   Serial.println("recv failed");
    // }
  }
  delay(10);
}
