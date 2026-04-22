#include <Arduino.h>
#include <RH_RF95.h>

#include "messages.h"

#include <SPI.h>
#include <Adafruit_Si7021.h>
#include <SparkFunMPL3115A2.h>

bool enableHeater = false;
uint8_t loopCnt = 0;

Adafruit_Si7021 sensor = Adafruit_Si7021();
MPL3115A2 myPressure;

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
  rf95.setTxPower(10);

  // Setup Spreading Factor
  rf95.setSpreadingFactor(8);

  // Setup BandWidth, option
  rf95.setSignalBandwidth(125000);

  // Setup Coding Rate:5(4/5),6(4/6),7(4/7),8(4/8)
  rf95.setCodingRate4(5);

  Serial.print("Listening on frequency: ");
  Serial.println(frequency);

if (!sensor.begin()) {
    Serial.println("Did not find Si7021 sensor!");
    while (true)
      ;
  }



  myPressure.begin(); // Get sensor online

  myPressure.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
  
  myPressure.setOversampleRate(7); // Set Oversample to the recommended 128
  myPressure.enableEventFlags(); // Enable all three pressure and temp event flags 


}

// struct __attribute__((packed)) msg_t {
//   uint8_t buf[RH_RF95_MAX_MESSAGE_LEN - sizeof(int) - sizeof(int16_t)];
//   int lastSNR;
//   int16_t lastRSSI;
// };

void loop() {
  // if (rf95.available()) {
  if (true) {
    /*uint8_t data[] = "english or spanish ?";
    rf95.send(data, sizeof(data));
    rf95.waitPacketSent();
    Serial.println("Sent a reply");*/
    // Should be a message for us now
    // uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    weather_data_t msg;
    msg.si7021_humidity = sensor.readHumidity();
    msg.si7021_temperature = sensor.readTemperature();
    msg.mpl_pressure = myPressure.readPressure();
    msg.mpl_temperature = myPressure.readTemp();

    

    // uint8_t *phrase = "salut";
    // memcpy(phrase, msg.buf, 6);
    // strcpy(phrase, msg.buf);
    uint8_t len = sizeof(msg);
    // if (rf95.recv((uint8_t *)&msg, &len)) {
    // RH_RF95::printBuffer("request: ", buf, len);
    // Serial.print("got request: ");
    // String a = String((char *)buf.buf);
    // Serial.println(a);
    // Serial.print((" with an SNR of: "));
    // Serial.print(rf95.lastSNR());
    // Serial.print((" an RRSI of: "));
    // Serial.println(rf95.lastRssi());

    // Send a reply
    // uint8_t data[] = "english or spanish ?";
    rf95.send((uint8_t *)&msg, sizeof(msg));
    Serial.println(("sent"));
    rf95.waitPacketSent();
    delay(1000);
    // Serial.println("Sent a reply");

    // } else {
    //   Serial.println("recv failed");
    // }

    if (++loopCnt == 30) {
    enableHeater = !enableHeater;
    sensor.heater(enableHeater);
    Serial.print("Heater Enabled State: ");
    if (sensor.isHeaterEnabled())
      Serial.println("ENABLED");
    else
      Serial.println("DISABLED");
       
    loopCnt = 0;
    }
  Serial.println("Waiting...");
  delay(100);
  }
}

