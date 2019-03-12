// Code is modified by https://MyElectronicslab.com original code written by RNL
#include <SoftwareSerial.h>

#include <Wire.h>
#include "RTClib.h"

// Configure software serial port
SoftwareSerial SIM900(7, 8);

#define TIMEOUT 5000
//#define DELAY 6000

int DELAY = 5000;

//Pin Sensor Kelembaban Tanah
const int soilMoistureSensor1Pin = A0;
const int soilMoistureSensor2Pin = A1;
float smSensor1Value = 0;
float smSensor2Value = 0;
float smSensorAverageValue = 0;


//Pin Sensor Ultrasonic/Jarak
const int pingPin = 4;
const int echo = 3;
long duration, inches, cm;

//Pin Aktuator Pompa
const int PROGMEM pumpPin = LED_BUILTIN;
//const int PROGMEM pumpPin = LED_BUILTIN;

// DS1307 RTC
// SDA pin to Arduino A4
// SCL pin to Arduino A5
RTC_DS1307 RTC;
int morningHour = 05;
DateTime now;


boolean isSMMonitoring = false;
boolean isWaterMonitoring = false;
long currentMillis;
long previousMillis;

String message;

boolean isWater = true;
String waterCondition;

boolean isNotification = false;


boolean isAutomatic = false;

char sms_text[100] = "";


// Variable to store text message
String textMessage;

// Create a variable to store Lamp state
String lampState = "HIGH";

const int ledBuiltin = LED_BUILTIN;

//String message;

// Relay connected to pin 3
const int relay = 3;


void setup() {
  // Automatically turn on the shield
  //  digitalWrite(9, HIGH);
  //  delay(1000);
  //  digitalWrite(9, LOW);
  //  delay(5000);

  //
  //
  //  //Menampilkan judul di Serial Monitor
  //  Serial.println("===Penyiram Tanaman Cabai===");
  //  Serial.println("Starting SIM800 Auto Read SMS");
  //
  //Memulai RTC
  Wire.begin();
  RTC.begin();
  if (!RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(2014, 1, 21, 4, 59, 49));// to set the time manualy
  }

  // By default the relay is off
  //  digitalWrite(relay, HIGH);

  // Initializing serial commmunication

  Serial.begin(19200);
  SIM900.begin(19200);

  // Give time to your GSM shield log on to network
  //  delay(20000);
  Serial.print("SIM900 is ready to send receive sms");

  // AT command to set SIM900 to SMS mode
  SIM900.print("AT+CMGF=1\r");
  delay(100);

  // Set module to send SMS data to serial out upon receipt
    SIM900.print("AT+CNMI=2,2,0,0,0\r");
    delay(100);



  // Set relay as OUTPUT
  pinMode(ledBuiltin, OUTPUT);
  //  pinMode(relay, OUTPUT);

  //Mengatur konfigurasi pin sensor Kelembaban Tanah
  pinMode(soilMoistureSensor1Pin, INPUT);
  pinMode(soilMoistureSensor2Pin, INPUT);

  //Mengatur konfigurasi pin sensor Ultrasonic
  pinMode(pingPin, OUTPUT);
  pinMode(echo, INPUT);

  //Mengatur konfigurasi pin Pompa
  pinMode(pumpPin, OUTPUT);

  //  sendSMS("Tes");
  //sendMessage();
}

// Function that sends SMS
void sendMessage(int mode)
{
  if (mode == 1) {
    Serial.println ("SIM900A Mengirim SMS");
    SIM900.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
    delay(1000);  // Delay of 1000 milli seconds or 1 second
    Serial.println ("Set SMS Number");
    SIM900.println("AT+CMGS=\"+6281327293085\"\r"); // Replace with your mobile number
    delay(1000);
    Serial.println ("Set SMS Content");
    SIM900.println(message);// The SMS text you want to send
    delay(100);
    Serial.println ("Finish");
    SIM900.println((char)26);// ASCII code of CTRL+Z
    delay(1000);
    Serial.println (" ->SMS Selesai dikirim");
  } else if (mode == 2) {
    Serial.println ("SIM900A Mengirim SMS");
    SIM900.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
    delay(1000);  // Delay of 1000 milli seconds or 1 second
    Serial.println ("Set SMS Number");
    SIM900.println("AT+CMGS=\"+6281327293085\"\r"); // Replace with your mobile number
    delay(1000);
    Serial.println ("Set SMS Content");
    SIM900.println(sms_text);// The SMS text you want to send
    delay(100);
    Serial.println ("Finish");
    SIM900.println((char)26);// ASCII code of CTRL+Z
    delay(1000);
    Serial.println (" ->SMS Selesai dikirim");
  }
}

//Tanpa Jeda
void pompaBuka() {
  digitalWrite(pumpPin, HIGH);
  sendMessage(1);
  //  gprs.sendSMS("+56955147132", "Pompa Dinyalakan");
}

void pompaTutup() {
  digitalWrite(pumpPin, LOW);
  sendMessage(1);
  //  gprs.sendSMS("+56955147132", "Pompa Dimatikan");
}

//Dengan Jeda
void jedaPompaBuka() {
  digitalWrite(pumpPin, HIGH);
  //  gprs.sendSMS("+56955147132", "Pompa Dinyalakan");
  message = "Pompa Dinyalakan";
  sendMessage(1);
  delay(DELAY);
  digitalWrite(pumpPin, LOW);
  message = "Pompa Dimatikan";
  sendMessage(1);
  //  gprs.sendSMS("+56955147132", "Pompa Dimatikan");
}


void getAllCondition() {
  char smSensorAverageValueChar[10];
  char cmChar[10];
  char copy[50];
  waterCondition.toCharArray(copy, 50);
  dtostrf(smSensorAverageValue, 1, 2, smSensorAverageValueChar); // float to string
  dtostrf(cm, 1, 2, cmChar); // float to string
  sprintf(sms_text, "Soil Moisture Average Value : %s, Jarak: %s cm, Keadaan Air : %s", smSensorAverageValueChar, cmChar, copy);
  //  gprs.sendSMS("+56955147132", sms_text);
  sendMessage(2);

  Serial.println(sms_text);
  Serial.println("\nSMS sent OK");
}

void getWaterCondition() {
  char cmChar[10];
  char copy[20];
  waterCondition.toCharArray(copy, 20);
  dtostrf(cm, 1, 2, cmChar); // float to string
  sprintf(sms_text, "Jarak: %s cm, Keadaan Air : %s", cmChar, copy);
  //  gprs.sendSMS("+56955147132", sms_text);
  sendMessage(2);
  Serial.println(sms_text);
  Serial.println("\nSMS sent OK");
}



long previousRTCMillis;
boolean isRTCShow = false;
//Menampilkan waktu
void showRTC() {
  if ((isRTCShow == false) && (currentMillis - previousRTCMillis >= 1000)) {
    now = RTC.now();
    Serial.print("DATE : ");
    Serial.print(now.day(), DEC);
    Serial.print("/");
    if (now.month() < 10) {
      Serial.print("0");
    }
    Serial.print(now.month(), DEC);
    Serial.print("/");

    Serial.println(now.year(), DEC);

    Serial.print("TIME : ");
    if (now.hour() < 10) {
      Serial.print("0");
    }
    Serial.print(now.hour(), DEC);
    Serial.print(":");
    if (now.minute() < 10) {
      Serial.print("0");
    }

    Serial.print(now.minute(), DEC);
    Serial.print(":");
    if (now.second() < 10) {
      Serial.print("0");
    }

    Serial.println(now.second(), DEC);
    //  delay(1000);
    isRTCShow = true;
    previousRTCMillis = currentMillis;
  } else if ((isRTCShow == true) && (currentMillis - previousRTCMillis >= 1000)) {
    isRTCShow = false;
    previousRTCMillis = currentMillis;
  }
}


void automatic() {

  //Alarm
  if ((int) morningHour != 0) {
    if (now.hour() == (int) morningHour && now.minute() == 0 && now.second() <= 30 ) {
      digitalWrite(pumpPin, HIGH);
    } else {
      digitalWrite(pumpPin, LOW);
    }
  }
}
//
//
long previousMonitorMillis;
//Semua Kondisi Sensor
void monitorAllCondition() {

  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  duration = pulseIn(echo, HIGH);

  //  inches = duration / 74 / 2;
  cm = duration / 29 / 2;

  //  Serial.print(inches);
  //  Serial.print("in, ");


  if ((isSMMonitoring == false) && (currentMillis - previousMonitorMillis >= 4000)) {
    smSensor1Value = analogRead(soilMoistureSensor1Pin);
    smSensor2Value = analogRead(soilMoistureSensor2Pin);
    smSensorAverageValue = (smSensor1Value + smSensor2Value) / 2;
    Serial.print("sensor1 : ");
    Serial.print(smSensor1Value);
    Serial.print(" & sensor2 : ");
    Serial.println(smSensor2Value);
    Serial.print("smSensorAverageValue : ");
    Serial.println(smSensorAverageValue);

    Serial.print(cm);
    Serial.print("cm");
    Serial.println();

    if (cm > 20) {
      Serial.println("Air Sudah Habis");
      waterCondition = "Air kering";
      digitalWrite(pumpPin, LOW);
      isWater = false;
    } else {
      Serial.println("Air Masih Ada");
      waterCondition = "Air masih ada";
      isWater = true;
    }

    isSMMonitoring = true;
    previousMonitorMillis = currentMillis;
  } else if ((isSMMonitoring == true) && (currentMillis - previousMonitorMillis >= 8000)) {
    isSMMonitoring = false;
    previousMonitorMillis = currentMillis;
  }

}

void loop() {

  //    now = RTC.now();
  //    Serial.print("DATE : ");
  //    Serial.print(now.day(), DEC);
  //    Serial.print("/");
  //    if (now.month() < 10) {
  //      Serial.print("0");
  //    }
  //    Serial.print(now.month(), DEC);
  //    Serial.print("/");
  //
  //    Serial.println(now.year(), DEC);
  //
  //    Serial.print("TIME : ");
  //    if (now.hour() < 10) {
  //      Serial.print("0");
  //    }
  //    Serial.print(now.hour(), DEC);
  //    Serial.print(":");
  //    if (now.minute() < 10) {
  //      Serial.print("0");
  //    }
  //
  //    Serial.print(now.minute(), DEC);
  //    Serial.print(":");
  //    if (now.second() < 10) {
  //      Serial.print("0");
  //    }
  //
  //    Serial.println(now.second(), DEC);
  //    delay(1000);

  //  currentMillis = millis();  //Sebagai stopwatch
  //  showRTC();
  //  monitorAllCondition();
  //      if (isAutomatic == true) {
  //        automatic();
  //      }

  //  if (isNotification) {
  //    if (isWater == false) {
  //      if ((isWaterMonitoring == false) && (currentMillis - previousWaterMillis >= 4000)) {
  //        gprs.sendSMS("+56955147132", "Air habis!!!");
  //        isWaterMonitoring = true;
  //        previousWaterMillis = currentMillis;
  //      } else if ((isWaterMonitoring == true) && (currentMillis - previousWaterMillis >= 64000)) {
  //        isWaterMonitoring = false;
  //        previousWaterMillis = currentMillis;
  //      }
  //    }
  //  }



  if (SIM900.available() > 0) {
    textMessage = SIM900.readString();
    Serial.print(textMessage);
    delay(10);
  }
  if (textMessage.indexOf("ON") >= 0) {
    // Turn on relay and save current state
    digitalWrite(pumpPin, HIGH);
    lampState = "on";
    Serial.println("Relay set to ON");
    textMessage = "";
//    SIM900.print("AT+CMGD=1,4");
//    delay(100);
  }
  if (textMessage.indexOf("OFF") >= 0) {
    // Turn off relay and save current state
    digitalWrite(pumpPin, LOW);
    lampState = "off";
    Serial.println("Relay set to OFF");
    textMessage = "";
//    SIM900.print("AT+CMGD=1,4");
//    delay(100);
  }
  if (textMessage.indexOf("STATE") >= 0) {
    message = "Lamp is " + lampState;
    //    sendSMS(message);
    sendMessage(1);
    Serial.println("Lamp state resquest");
    textMessage = "";
  }


  //    if (textMessage.indexOf("ACTION_START") >= 0) {
  //      // Turn off relay and save current state
  //      message = "POMPA ON";
  //      pompaBuka();
  //      lampState = "on";
  //
  //      Serial.println("PUMP ACTIVATED");
  //      textMessage = "";
  //    }
  //
  //    if (textMessage.indexOf("ACTION_STOP") >= 0) {
  //      // Turn off relay and save current state
  //      message = "POMPA OFF";
  //      pompaTutup();
  //      lampState = "off";
  //      Serial.println("PUMP DEACTIVATED");
  //      textMessage = "";
  //    }
  //
  //    if (textMessage.indexOf("ACTION_DELAYSTART") >= 0) {
  //      // Turn off relay and save current state
  //      lampState = "on";
  //      Serial.println("DELAY PUMP ACTIVATED");
  //      textMessage = "";
  //      jedaPompaBuka();
  //    }
  //    if (textMessage.indexOf("GET_ALLCONDITION") >= 0) {
  //      // Turn off relay and save current state
  //      Serial.println("GET ALL CONDTION");
  //      textMessage = "";
  //      getAllCondition();
  //    }
  //    if (textMessage.indexOf("GET_WATERCONDITION") >= 0) {
  //      // Turn off relay and save current state
  //      Serial.println("GET WATER CONDTION");
  //      textMessage = "";
  //      getWaterCondition();
  //    }
  //
  //    if (textMessage.indexOf("#") >= 0) {
  //      // Turn off relay and save current state
  //      Serial.println("ENABLE AUTOMATIC");
  //      textMessage = "";
  //      isAutomatic = true;
  //    }
  //    if (textMessage.indexOf("$") >= 0) {
  //      // Turn off relay and save current state
  //      Serial.println("DISABLE AUTOMATIC");
  //      textMessage = "";
  //      isAutomatic = false;
  //    }
}



