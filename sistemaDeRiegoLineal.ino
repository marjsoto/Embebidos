#include <AirQualityClass.h>
#include <Arduino_MKRIoTCarrier.h>
#include <Arduino_MKRIoTCarrier_Buzzer.h>
#include <Arduino_MKRIoTCarrier_Qtouch.h>
#include <Arduino_MKRIoTCarrier_Relay.h>
#include <EnvClass.h>
#include <IMUClass.h>
#include <MKRIoTCarrierDefines.h>
#include <PressureClass.h>

#include <Arduino_MKRIoTCarrier.h>
#include <Servo.h>
#include "visuals.h"
#include "thingProperties.h"
MKRIoTCarrier carrier;
Servo servo1;
int servoPin = 5;
int humidityPin1;
int humidityPin2;
String waterPumpState;
String lightState;
uint32_t lightsOn = carrier.leds.Color(143, 0, 255);
uint32_t lightsOff = carrier.leds.Color(0, 0, 0);
 
void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  delay(500);
  initProperties();
 
  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  //Get Cloud Info/errors , 0 (only errors) up to 4
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
 
  //Wait to get cloud connection to init the carrier
  while (ArduinoCloud.connected() != 1) {
    ArduinoCloud.update();
    delay(500);
  }
  delay(500);
  CARRIER_CASE = false;
  carrier.begin();
  servo1.attach(servoPin);
  humidityPin1 = carrier.getBoardRevision() == 1 ? A5 : A0; //assign A0 or A5 based on HW revision
  humidityPin2 = A6;
  carrier.display.setRotation(0);
  carrier.display.fillScreen(0x0000);
  //Basic configuration for the text
  carrier.display.setRotation(0);
  carrier.display.setTextWrap(true);
  carrier.display.drawBitmap(50, 40, arduino_logo, 150, 106, 0x253); //0x00979D);
  carrier.display.setTextColor(0xFFFF);
  carrier.display.setTextSize(3);
  carrier.display.setCursor(35, 160);
  carrier.display.print("Loading");
  for (int i = 0; i < 3; i++)
  {
    carrier.display.print(".");
    delay(1000);
  }
}
 
void loop() {
  ArduinoCloud.update();
  temperature = carrier.Env.readTemperature();
  int raw_humidity1 = analogRead(humidityPin1);
  humidity1 = map(raw_humidity1, 0, 1023, 100, 0);
  int raw_humidity2 = analogRead(humidityPin2);
  humidity2 = map(raw_humidity2, 0, 1023, 100, 0);
  while (!carrier.Light.colorAvailable()); 
  int none;
  carrier.Light.readColor(none, none, none, light);\
  onArtificialLightChange();
  onWaterpumpChange();
  updateScreen(); // Actualizar la pantalla en cada ciclo
  
  delay(1000);
  
}
 
 
void onWaterpumpChange() {
  ArduinoCloud.update();
  if (/*humidity2<15 || */waterpump2==true) {
    delay(1000);
    servo1.write(0);
    carrier.Relay1.close();
    waterPumpState = "PUMP: ON";
    delay(3000);
    carrier.Relay1.open();
  }else if (/*humidity1<15 || */waterpump1==true) {
    servo1.write(0);
    delay(5000);
    servo1.write(30/3);
    delay(1000);
    servo1.write(60/3);
    delay(1000);
    servo1.write(90/3);
    delay(1000);
    carrier.Relay1.close();
    waterPumpState = "PUMP: ON";
    delay(3000);
    carrier.Relay1.open();
  }  else {
    servo1.write(0);
    carrier.Relay1.open();
    waterPumpState = "PUMP: OFF";
  }
  updateScreen();
}

void onArtificialLightChange() {
  ArduinoCloud.update();
  if (/*light<150 || */artificial_light == true) {
    uint8_t b = 50;
    carrier.leds.setBrightness(b);
    carrier.leds.fill(lightsOn, 0, 5);
    carrier.leds.show();    
    lightState = "LIGHTS: ON";
  } else {
    carrier.leds.fill(lightsOff, 0, 5);
    carrier.leds.show();
    lightState = "LIGHTS: OFF";
  }
  updateScreen();
}
 
//Update displayed Info
void updateScreen() {
  
  carrier.display.fillScreen(0x0000);
  carrier.display.setTextColor(0xFFFF);
  carrier.display.setTextSize(2); // Tamaño de letra más pequeño para que quepan los valores
  // Mostrar valores de los sensores
  carrier.display.drawBitmap(20, 50, humidity_logo, 20, 20, 0x0D14);
  carrier.display.setCursor(40, 50);
  carrier.display.print("Hum1: ");
  carrier.display.print(humidity1);
  carrier.display.print("%");
  carrier.display.drawBitmap(20, 70, temperature_logo, 20, 20, 0x0000ff); //0xDA5B4A); //draw a thermometer on the MKR IoT carrier's display
  carrier.display.setCursor(40, 70);
  carrier.display.print("Temp: ");
  carrier.display.print(temperature);
  carrier.display.print("C");
  carrier.display.drawBitmap(5, 90, luz_logo, 20, 20, 0xFFFF00);
  carrier.display.setCursor(30, 90);
  carrier.display.print("Luz: ");
  carrier.display.print(light);
  carrier.display.drawBitmap(20, 110, humidity_logo, 20, 20, 0x0D14);
  carrier.display.setCursor(40, 110);
  carrier.display.print("Hum2: ");
  carrier.display.print(humidity2);
  carrier.display.print("%");
  // Mostrar estados de los actuadores (puedes ajustar la posición si es necesario)
  carrier.display.drawBitmap(10, 120, pump_logo, 30, 30, 0x0D14);
  carrier.display.setCursor(40, 130);
  carrier.display.print(waterPumpState);
  carrier.display.drawBitmap(30, 150, luz_logo, 20, 20, 0xFFFF00);
  carrier.display.setCursor(50, 150);
  carrier.display.print(lightState);
}
