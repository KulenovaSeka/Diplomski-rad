#include "IoTaaP.h"
#include "IoTaaP_HAPI.h"
#include "BlynkSimpleEsp32.h"
#include <Servo.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <WiFi.h> 

#define BLYNK_PRINT Serial
#define BLYNK_GREEN  "#23C48E"
#define BLYNK_RED    "#D3435C"

const int oneWireBus = 26;                              //SPAJANJE TEMPERATURNOG SENZORA NA ESP32
const int trigPin = 12;                                 //SPAJANJE SENZORA UDALJENOSTI NA ESP32
const int echoPin = 13;                                 //SPAJANJE SENZORA UDALJENOSTI NA ESP32
const int Napajanje_centralnog_grijanja = 27;           //SPAJANJE RELEJA NA ESP32
const int Centralno = 15;                               //SPAJANJE SERVO MOTORA NA ESP32
const int Vrata = 4;                                    //SPAJANJE SERVO MOTORA NA ESP32

OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire); 
IoTaaP iotaap;
WidgetLED led1(V0);
BlynkTimer timer;
Servo Centralno_paljenje;
Servo Garazna_vrata;

int pos1=36;                                            //POZICIJA SERVO MOTORA U STUPNJEVIMA
int pos2=90;                                           //POZICIJA SERVO MOTORA U STUPNJEVIMA
int pos11=155;                                          //POZICIJA SERVO MOTORA U STUPNJEVIMA
int pos3=18;                                            //POZICIJA SERVO MOTORA U STUPNJEVIMA
int pos4=0;                                             //POZICIJA SERVO MOTORA U STUPNJEVIMA
int ledState = LOW;
long duration;
int distance;
int postotak;
float temperatureC;
unsigned long previousMillis = 0;
unsigned long interval = 30000;

char auth[] = "B5uq_Mp1PhGAC92CtAVSBwXua8CACfmx"; //TOKEN DOBIVEN IZ BLYNK MOBILNE APLIKACIJE
char ssid[] = "Horvat_EXT";                       //POSTAVKE WiFi KUĆNE MREŽE
char pass[] = "KulenovaSeka";                     //SPAJANJE NA WiFi EKSTENDER POSTAVLJEN U GARAŽI


BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1, V5, V6);            //SINKRONIZACIJA PODATAKA SA SERVEROM USLIJED NESTANKA NAPAJANJA
}

BLYNK_WRITE(V1)                                   //UPRAVLJANJE NAPAJANJEM CENTRALNOG GRIJANJA
{
  ledState = param.asInt();
  digitalWrite(Napajanje_centralnog_grijanja, ledState); 
}

BLYNK_WRITE(V2)                                   //UPRAVLJANJE POKRETANJA CENTRALNOG GRIJANJA
{
   Centralno_paljenje.write(pos1);              
   delay(1900); 
   led1.setColor(BLYNK_GREEN);
   led1.on();                       
   Centralno_paljenje.write(pos2);           
}

BLYNK_WRITE(V3)                                   //UPRAVLJANJE GAŠENJA CENTRALNOG GRIJANJA
{  
   Centralno_paljenje.write(pos11);              
   delay(1800);
   led1.setColor(BLYNK_RED); 
   led1.on();                        
   Centralno_paljenje.write(pos2);
}

BLYNK_WRITE(V4)                                   //UPRAVLJANJE GARAŽNIM VRATIMA
{
    Garazna_vrata.write(pos3);              
    delay(800);  
    Garazna_vrata.write(pos4);          
}

void MjerenjeUdaljenosti();                       //FUNKCIJA ZA MJERENJE UDALJENOSTI
void MjerenjeTemp();                              //FUNKCIJA ZA MJERENJE TEMPERATURE

void myTimerEvent()                               //SLANJE PODATAKA NA BLYNK MOBILNU APLIKACIJU 
{
  Blynk.virtualWrite(V5,postotak);
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
  Serial.print('.');
  delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(9600);
  initWiFi();
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Centralno_paljenje.attach(Centralno);
  Garazna_vrata.attach(Vrata);
  Blynk.begin(auth, ssid, pass);
  pinMode(Napajanje_centralnog_grijanja,OUTPUT);
  digitalWrite(Napajanje_centralnog_grijanja,ledState);
  timer.setInterval(20000L, myTimerEvent);         //SLANJE PODATAKA U POSTOKU PELETA I TEMPERATURI KOTLA SVAKIH 20 SEC
  }

  void loop()
  {
    unsigned long currentMillis = millis();
    if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }
    Blynk.run();
    timer.run();                                   //POKRETANJE BLYNK TIMERA
    MjerenjeUdaljenosti();
    MjerenjeTemp();
    delay(2000);
  }

    void Prazan(){
      Blynk.notify("Prazan spremnik peleta");      //SLANJE NOTIFIKACIJE NA MOBITEL O PRAZNOM SPREMNIKU PELETA
    }

  void Visoka(){
      Blynk.notify("Visoka temperatura u kotlu");  //SLANJE NOTIFIKACIJE NA MOBITEL O VISOKOJ TEMPERATURI KOTLA
    }  

  void MjerenjeTemp(){                             //FUNKCIJA MJERENJA TEMPERATURE VODE
      sensors.requestTemperatures(); 
      temperatureC = sensors.getTempCByIndex(0)+3;
      Serial.println("Temperatura vode:" + (String)temperatureC+ "ºC");
      if (temperatureC>=70){
      Visoka();
    }
  } 
  void MjerenjeUdaljenosti(){                     //FUNKCIJA MJERENJA UDALJENOSTI
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance =duration*0.034/2;
    if (distance<=1)
    {
      postotak=100;
    }
    if (distance>=1.1 && distance<=2.1)
    {
      postotak=95;
    }
    if (distance>=2.2 && distance<=4.3)
    {
      postotak=90;
    }
    if (distance>=4.4 && distance<=6.5)
    {
      postotak=85;
    }
    if (distance>=6.6 && distance<=8.7)
    {
      postotak=80;
    }
    if (distance>=8.8 && distance<=10.9)
    {
      postotak=75;
    }
    if (distance>=11 && distance<=13.1)
    {
      postotak=70;
    }
    if (distance>=13.2 && distance<=15.3)
    {
      postotak=65;
    }
    if (distance>=15.4 && distance<=17.5)
    {
      postotak=60;
    }
    if (distance>=17.6 && distance<=19.8)
    {
      postotak=55;
    }
    if (distance>=19.8 && distance<=21.9)
    {
      postotak=50;
    }
    if (distance>=22 && distance<=24.1)
    {
      postotak=45;
    }
    if (distance>=24.2 && distance<=26.3)
    {
      postotak=40;
    }
    if (distance>=26.4 && distance<=28.5)
    {
      postotak=35;
    }
    if (distance>=28.6 && distance<=30.7)
    {
      postotak=30;
    }
    if (distance>=30.8 && distance<=31.9)
    {
      postotak=25;
    }
    if (distance>=33 && distance<=35.1)
    {
      postotak=20;
    }
    if (distance>=35.2 && distance<=37.3)
    {
      postotak=15;
    }
    if (distance>=37.4 && distance<=39.5)
    {
      postotak=10;
    }
    if (distance>=39.6 && distance<=41.7)
    {
      postotak=5;
      Prazan();
    }
        if (distance>=41.8 && distance<=44)
    {
      postotak=0;
      Prazan();
    }
    if (distance>= 50)
  {
    postotak=0;
  }
  Serial.println("Razina peleta:" + (String)distance+ " cm");
  }