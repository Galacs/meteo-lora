#include <Arduino.h>
#include "messages.h"
#include <SPI.h>
#include <SparkFun_Weather_Meter_Kit_Arduino_Library.h>
#include <MS5xxx.h>
#include <Adafruit_SHT31.h>
#include <Wire.h>
#include <SPI.h>
#include <HTU21D.h>
#include <RadioLib.h>
#include <sys/unistd.h>
#include <errno.h>
#include <Adafruit_BQ25798.h>

#define LORA_RST_PIN PB0
#define LORA_DIO1_PIN PB1
#define LORA_BUSY_PIN PB11
#define LORA_DIO3_PIN PB10
#define LORA_CS_PIN PA4
#define LORA_SCK_PIN PA5
#define LORA_MISO_PIN PA6
#define LORA_MOSI PA7


SX1262 radio = new Module(LORA_CS_PIN, LORA_DIO1_PIN, LORA_RST_PIN, LORA_BUSY_PIN);

#define BTN_PIN PB12
#define STATUS_PIN PB13

#define SCL_PIN PB6
#define SDA_PIN PB7
#define CE_PIN PA11
#define INT_PIN PA12
#define QON_PIN PA10

#define RGB_PIN PC13

#define rainfallPin PB4
#define windDirectionPin PA1
#define windSpeedPin PA0

/*
HTU21D(resolution)

resolution:
HTU21D_RES_RH12_TEMP14 - RH: 12Bit, Temperature: 14Bit, by default
HTU21D_RES_RH8_TEMP12  - RH: 8Bit,  Temperature: 12Bit
HTU21D_RES_RH10_TEMP13 - RH: 10Bit, Temperature: 13Bit
HTU21D_RES_RH11_TEMP11 - RH: 11Bit, Temperature: 11Bit
*/
HTU21D myHTU21D(HTU21D_RES_RH12_TEMP14);
MS5xxx MS5607(&Wire);
Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_BQ25798 bq;

bool enableHeater = false;
uint8_t loopCnt = 0;

SFEWeatherMeterKit weatherMeterKit(windDirectionPin, windSpeedPin, rainfallPin);
// Singleton instance of the radio driver

void setup() {
  SPI.setMISO(LORA_MISO_PIN);
  SPI.setMOSI(LORA_MOSI);
  SPI.setSCLK(LORA_SCK_PIN);

  ConfigLoRa_t config;
  config.frequency = 868.1;
  config.codingRate = 5;
  config.spreadingFactor = 8;
  config.power = 22;

  int state = radio.begin(config);
  if (state == RADIOLIB_ERR_NONE) {
    printf("lora success!\n");
  } else {
    printf("failed, code: %d \n", state);

    while (true) { delay(10); }
  }

  weatherMeterKit.begin();

  Wire.setSCL(SCL_PIN);
  Wire.setSDA(SDA_PIN);
  Wire.begin();

  MS5607.setI2Caddr(0x77);
  if (!MS5607.connect())
  {
    printf("MS5607 found.\n");
    // printf("%d\n", MS5607.getAddress());
  }
  else
  {
    printf("MS5607 not found. halt.\n");
    while (1);
  }

   while (myHTU21D.begin() != true)
  {
    printf("HTU21D, SHT21 sensor is faild or not connected\n"); //(F()) saves string to flash & keeps dynamic memory free
    delay(5000);
  }

    if (!sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
      printf("Couldn't find SHT31\n");
    while (1) delay(1);
  }

  printf("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    printf("ENABLED\n");
  else
    printf("DISABLED\n");

  printf("HTU21D, SHT21 sensor is active \n");
  pinMode(STATUS_PIN, OUTPUT);

  // Charger
  // if (!bq.begin()) {
  //   printf("Could not find a valid BQ25798 sensor, check wiring!\n");
  //   while (1);
  // }
  // printf("BQ25798 found!\n");
  // char bqbuf[10];

  // dtostrf(bq.getMinSystemV(), 3, 2, bqbuf);
  // printf("Minimal system voltage: %s V\n", bqbuf);

  // dtostrf(bq.getChargeLimitV(), 3, 2, bqbuf);
  // printf("Charge voltage limit: %s V\n", bqbuf);

  // dtostrf(bq.getChargeLimitA(), 3, 2, bqbuf);
  // printf("Charge current limit: %s A\n", bqbuf);

  // dtostrf(bq.getInputLimitV(), 3, 2, bqbuf);
  // printf("Input voltage limit: %s V\n", bqbuf);

  // dtostrf(bq.getInputLimitA(), 3, 2, bqbuf);
  // printf("Input current limit: %s A\n", bqbuf);

  // dtostrf(bq.getPrechargeLimitA(), 3, 2, bqbuf);
  // printf("Precharge current limit: %s A\n", bqbuf);

  // dtostrf(bq.getTerminationA(), 3, 2, bqbuf);
  // printf("Termination current limit: %s A\n", bqbuf);

  // dtostrf(bq.getRechargeThreshOffsetV(), 3, 2, bqbuf);
  // printf("Recharge threshold offset: %s V (below VREG)\n", bqbuf);

  // dtostrf(bq.getOTGV(), 3, 2, bqbuf);
  // printf("OTG voltage: %s V\n", bqbuf);

  // dtostrf(bq.getOTGLimitA(), 3, 2, bqbuf);
  // printf("OTG current limit: %s A\n", bqbuf);

  // printf("VBat low threshold: ");
  // switch (bq.getVBatLowV()) {
  //   case BQ25798_VBAT_LOWV_15_PERCENT:   printf("15%% of VREG\n"); break;
  //   case BQ25798_VBAT_LOWV_62_2_PERCENT: printf("62.2%% of VREG\n"); break;
  //   case BQ25798_VBAT_LOWV_66_7_PERCENT: printf("66.7%% of VREG\n"); break;
  //   case BQ25798_VBAT_LOWV_71_4_PERCENT: printf("71.4%% of VREG\n"); break;
  // }

  // printf("Cell count: ");
  // switch (bq.getCellCount()) {
  //   case BQ25798_CELL_COUNT_1S: printf("1S (1 cell)\n"); break;
  //   case BQ25798_CELL_COUNT_2S: printf("2S (2 cells)\n"); break;
  //   case BQ25798_CELL_COUNT_3S: printf("3S (3 cells)\n"); break;
  //   case BQ25798_CELL_COUNT_4S: printf("4S (4 cells)\n"); break;
  // }

  // printf("Recharge deglitch time: ");
  // switch (bq.getRechargeDeglitchTime()) {
  //   case BQ25798_TRECHG_64MS:   printf("64ms\n"); break;
  //   case BQ25798_TRECHG_256MS:  printf("256ms\n"); break;
  //   case BQ25798_TRECHG_1024MS: printf("1024ms\n"); break;
  //   case BQ25798_TRECHG_2048MS: printf("2048ms\n"); break;
  // }

  // printf("Precharge timer: ");
  // switch (bq.getPrechargeTimer()) {
  //   case BQ25798_PRECHG_TMR_2HR:   printf("2 hours\n"); break;
  //   case BQ25798_PRECHG_TMR_0_5HR: printf("0.5 hours\n"); break;
  // }

  // printf("Top-off timer: ");
  // switch (bq.getTopOffTimer()) {
  //   case BQ25798_TOPOFF_TMR_DISABLED: printf("Disabled\n"); break;
  //   case BQ25798_TOPOFF_TMR_15MIN:    printf("15 minutes\n"); break;
  //   case BQ25798_TOPOFF_TMR_30MIN:    printf("30 minutes\n"); break;
  //   case BQ25798_TOPOFF_TMR_45MIN:    printf("45 minutes\n"); break;
  // }

  // printf("Fast charge timer: ");
  // switch (bq.getFastChargeTimer()) {
  //   case BQ25798_CHG_TMR_5HR:  printf("5 hours\n"); break;
  //   case BQ25798_CHG_TMR_8HR:  printf("8 hours\n"); break;
  //   case BQ25798_CHG_TMR_12HR: printf("12 hours\n"); break;
  //   case BQ25798_CHG_TMR_24HR: printf("24 hours\n"); break;
  // }

  // printf("Trickle charge timer enabled: %s\n", bq.getTrickleChargeTimerEnable() ? "true" : "false");
  // printf("Precharge timer enabled: %s\n", bq.getPrechargeTimerEnable() ? "true" : "false");
  // printf("Fast charge timer enabled: %s\n", bq.getFastChargeTimerEnable() ? "true" : "false");
  // printf("Timer half-rate enabled: %s\n", bq.getTimerHalfRateEnable() ? "true" : "false");
  // printf("Auto OVP battery discharge enabled: %s\n", bq.getAutoOVPBattDischarge() ? "true" : "false");
  // printf("Force battery discharge enabled: %s\n", bq.getForceBattDischarge() ? "true" : "false");
  // printf("Charge enabled: %s\n", bq.getChargeEnable() ? "true" : "false");
  // printf("ICO enabled: %s\n", bq.getICOEnable() ? "true" : "false");
  // printf("Force ICO enabled: %s\n", bq.getForceICO() ? "true" : "false");
  // printf("HIZ mode enabled: %s\n", bq.getHIZMode() ? "true" : "false");
  // printf("Termination enabled: %s\n", bq.getTerminationEnable() ? "true" : "false");
  // printf("Backup mode enabled: %s\n", bq.getBackupModeEnable() ? "true" : "false");

  // printf("Backup mode threshold: ");
  // switch (bq.getBackupModeThresh()) {
  //   case BQ25798_VBUS_BACKUP_40_PERCENT:  printf("40%% of VINDPM\n"); break;
  //   case BQ25798_VBUS_BACKUP_60_PERCENT:  printf("60%% of VINDPM\n"); break;
  //   case BQ25798_VBUS_BACKUP_80_PERCENT:  printf("80%% of VINDPM\n"); break;
  //   case BQ25798_VBUS_BACKUP_100_PERCENT: printf("100%% of VINDPM\n"); break;
  // }

  // printf("VAC OVP threshold: ");
  // switch (bq.getVACOVP()) {
  //   case BQ25798_VAC_OVP_26V: printf("26V\n"); break;
  //   case BQ25798_VAC_OVP_22V: printf("22V\n"); break;
  //   case BQ25798_VAC_OVP_12V: printf("12V\n"); break;
  //   case BQ25798_VAC_OVP_7V:  printf("7V\n"); break;
  // }

  // printf("Watchdog timer: ");
  // switch (bq.getWDT()) {
  //   case BQ25798_WDT_DISABLE: printf("Disabled\n"); break;
  //   case BQ25798_WDT_0_5S:    printf("0.5 seconds\n"); break;
  //   case BQ25798_WDT_1S:      printf("1 second\n"); break;
  //   case BQ25798_WDT_2S:      printf("2 seconds\n"); break;
  //   case BQ25798_WDT_20S:     printf("20 seconds\n"); break;
  //   case BQ25798_WDT_40S:     printf("40 seconds\n"); break;
  //   case BQ25798_WDT_80S:     printf("80 seconds\n"); break;
  //   case BQ25798_WDT_160S:    printf("160 seconds\n"); break;
  // }

  // printf("Force D+/D- pins detection enabled: %s\n", bq.getForceDPinsDetection() ? "true" : "false");
  // printf("Auto D+/D- pins detection enabled: %s\n", bq.getAutoDPinsDetection() ? "true" : "false");
  // printf("HVDCP 12V enabled: %s\n", bq.getHVDCP12VEnable() ? "true" : "false");
  // printf("HVDCP 9V enabled: %s\n", bq.getHVDCP9VEnable() ? "true" : "false");
  // printf("HVDCP enabled: %s\n", bq.getHVDCPEnable() ? "true" : "false");

  // printf("Ship FET mode: ");
  // switch (bq.getShipFETmode()) {
  //   case BQ25798_SDRV_IDLE:         printf("IDLE\n"); break;
  //   case BQ25798_SDRV_SHUTDOWN:     printf("Shutdown Mode\n"); break;
  //   case BQ25798_SDRV_SHIP:         printf("Ship Mode\n"); break;
  //   case BQ25798_SDRV_SYSTEM_RESET: printf("System Power Reset\n"); break;
  // }

  // printf("Ship FET 10s delay enabled: %s\n", bq.getShipFET10sDelay() ? "true" : "false");
  // printf("AC enabled: %s\n", bq.getACenable() ? "true" : "false");
  // printf("OTG enabled: %s\n", bq.getOTGenable() ? "true" : "false");
  // printf("OTG PFM enabled: %s\n", bq.getOTGPFM() ? "true" : "false");
  // printf("Forward PFM enabled: %s\n", bq.getForwardPFM() ? "true" : "false");

  // printf("Ship wakeup delay: ");
  // switch (bq.getShipWakeupDelay()) {
  //   case BQ25798_WKUP_DLY_1S:   printf("1 second\n"); break;
  //   case BQ25798_WKUP_DLY_15MS: printf("15ms\n"); break;
  // }

  // printf("BATFET LDO precharge enabled: %s\n", bq.getBATFETLDOprecharge() ? "true" : "false");
  // printf("OTG OOA enabled: %s\n", bq.getOTGOOA() ? "true" : "false");
  // printf("Forward OOA enabled: %s\n", bq.getForwardOOA() ? "true" : "false");
  // printf("ACDRV2 enabled: %s\n", bq.getACDRV2enable() ? "true" : "false");
  // printf("ACDRV1 enabled: %s\n", bq.getACDRV1enable() ? "true" : "false");

  // printf("PWM frequency: ");
  // switch (bq.getPWMFrequency()) {
  //   case BQ25798_PWM_FREQ_1_5MHZ: printf("1.5 MHz\n"); break;
  //   case BQ25798_PWM_FREQ_750KHZ: printf("750 kHz\n"); break;
  // }

  // printf("STAT pin enabled: %s\n", bq.getStatPinEnable() ? "true" : "false");
  // printf("VSYS short protection enabled: %s\n", bq.getVSYSshortProtect() ? "true" : "false");
  // printf("VOTG UVP protection enabled: %s\n", bq.getVOTG_UVPProtect() ? "true" : "false");
  // printf("IBUS OCP enabled: %s\n", bq.getIBUS_OCPenable() ? "true" : "false");
  // printf("VINDPM detection enabled: %s\n", bq.getVINDPMdetection() ? "true" : "false");
  // printf("Ship FET present: %s\n", bq.getShipFETpresent() ? "true" : "false");
  // printf("Battery discharge sense enabled: %s\n", bq.getBatDischargeSenseEnable() ? "true" : "false");

  // printf("Battery discharge regulation: ");
  // switch (bq.getBatDischargeA()) {
  //   case BQ25798_IBAT_REG_3A:      printf("3A\n"); break;
  //   case BQ25798_IBAT_REG_4A:      printf("4A\n"); break;
  //   case BQ25798_IBAT_REG_5A:      printf("5A\n"); break;
  //   case BQ25798_IBAT_REG_DISABLE: printf("Disabled\n"); break;
  // }

  // printf("IINDPM enabled: %s\n", bq.getIINDPMenable() ? "true" : "false");
  // printf("External ILIM pin enabled: %s\n", bq.getExtILIMpin() ? "true" : "false");
  // printf("Battery discharge OCP enabled: %s\n", bq.getBatDischargeOCPenable() ? "true" : "false");

  // printf("VINDPM VOC percentage: ");
  // switch (bq.getVINDPM_VOCpercent()) {
  //   case BQ25798_VOC_PCT_56_25: printf("56.25%%\n"); break;
  //   case BQ25798_VOC_PCT_62_5:  printf("62.5%%\n"); break;
  //   case BQ25798_VOC_PCT_68_75: printf("68.75%%\n"); break;
  //   case BQ25798_VOC_PCT_75:    printf("75%%\n"); break;
  //   case BQ25798_VOC_PCT_81_25: printf("81.25%%\n"); break;
  //   case BQ25798_VOC_PCT_87_5:  printf("87.5%%\n"); break;
  //   case BQ25798_VOC_PCT_93_75: printf("93.75%%\n"); break;
  //   case BQ25798_VOC_PCT_100:   printf("100%%\n"); break;
  // }

  // printf("VOC delay: ");
  // switch (bq.getVOCdelay()) {
  //   case BQ25798_VOC_DLY_50MS:  printf("50ms\n"); break;
  //   case BQ25798_VOC_DLY_300MS: printf("300ms\n"); break;
  //   case BQ25798_VOC_DLY_2S:    printf("2 seconds\n"); break;
  //   case BQ25798_VOC_DLY_5S:    printf("5 seconds\n"); break;
  // }

  // printf("VOC measurement rate: ");
  // switch (bq.getVOCrate()) {
  //   case BQ25798_VOC_RATE_30S:  printf("30 seconds\n"); break;
  //   case BQ25798_VOC_RATE_2MIN: printf("2 minutes\n"); break;
  //   case BQ25798_VOC_RATE_10MIN: printf("10 minutes\n"); break;
  //   case BQ25798_VOC_RATE_30MIN: printf("30 minutes\n"); break;
  // }

  // printf("MPPT enabled: %s\n", bq.getMPPTenable() ? "true" : "false");

  // printf("Thermal regulation threshold: ");
  // switch (bq.getThermRegulationThresh()) {
  //   case BQ25798_TREG_60C:  printf("60C\n"); break;
  //   case BQ25798_TREG_80C:  printf("80C\n"); break;
  //   case BQ25798_TREG_100C: printf("100C\n"); break;
  //   case BQ25798_TREG_120C: printf("120C\n"); break;
  // }

  // printf("Thermal shutdown threshold: ");
  // switch (bq.getThermShutdownThresh()) {
  //   case BQ25798_TSHUT_150C: printf("150C\n"); break;
  //   case BQ25798_TSHUT_130C: printf("130C\n"); break;
  //   case BQ25798_TSHUT_120C: printf("120C\n"); break;
  //   case BQ25798_TSHUT_85C:  printf("85C\n"); break;
  // }

  // printf("VBUS pulldown enabled: %s\n", bq.getVBUSpulldown() ? "true" : "false");
  // printf("VAC1 pulldown enabled: %s\n", bq.getVAC1pulldown() ? "true" : "false");
  // printf("VAC2 pulldown enabled: %s\n", bq.getVAC2pulldown() ? "true" : "false");
  // printf("Backup ACFET1 on: %s\n", bq.getBackupACFET1on() ? "true" : "false");

  // printf("--- End BQ25798 Status ---\n\n");

}


extern "C" int _write(int file, char *data, int len) {
  if ((file != STDOUT_FILENO) && (file != STDERR_FILENO)) {
    errno = EBADF;
    return -1;
  }

  for (int i = 0; i < len; i++) {
    ITM_SendChar(data[i]);
  }
  return len;
  }
bool a = false;
void loop() {
  if (true) {
    MS5607.ReadProm();
    MS5607.Readout();
    weather_data_t msg;
    msg.wind_direction = weatherMeterKit.getWindDirection();
    msg.wind_speed = weatherMeterKit.getWindSpeed();
    msg.rain_fall = weatherMeterKit.getTotalRainfall();
    msg.MS5607_temperature = MS5607.GetTemp();
    msg.MS5607_pressure = MS5607.GetPres();
    msg.HTU21D_temperature = myHTU21D.readTemperature();
    msg.HTU21D_humidity = myHTU21D.readHumidity();
    msg.HTU21D_compensed_humidity = myHTU21D.readCompensatedHumidity();
    msg.GXHT30_temperature = sht31.readTemperature();
    msg.GXHT30_humidity = sht31.readHumidity();

    char buf[10];
    dtostrf(msg.MS5607_temperature, 3, 2, buf);
    printf("a: %s\n", buf);
    dtostrf(msg.wind_speed, 3, 2, buf);
    printf("a: %s\n", buf);
    dtostrf(msg.HTU21D_humidity, 3, 2, buf);
    printf("a: %s\n", buf);
    dtostrf(msg.HTU21D_temperature, 3, 2, buf);
    printf("a: %s\n", buf);
    dtostrf(msg.GXHT30_temperature, 3, 2, buf);
    printf("a: %s\n", buf);

    radio.transmit((uint8_t *)&msg, sizeof(msg));
  
    printf("sent \n");
    delay(5000);
    digitalWrite(STATUS_PIN, a);
    a != a;
    delay(5000);
  }
}
