#include <SDL_Weather_80422.h>

#include "Adafruit_BMP085.h"
#include <Wire.h>
#include <Ethernet.h>
#include <SPI.h>
#include <DHT.h>
#include <Time.h>
//#include "SDL_Weather_80422.h"
#define PIN_ANEMOMETER  2     // Digital 2
#define PIN_VANE        0     // Analog 0
#define PIN_SOLARCELL   1
#define PIN_CHUV 3
#define DHTPIN 4
#define DHTTYPE DHT11

Adafruit_BMP085 bmp180;
int pchuv=0, pvent=0;
double temp, pres,umid=0;
int dir[16];
char server[] = "172.16.150.1";
IPAddress ip(172, 16, 103, 192);
byte gateway[] = {172, 16, 150, 250};
byte ns[] = {172, 16, 11, 1};
byte subnet[] = {255, 255, 255, 0};
int irradSolar = 0;

EthernetClient client;
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0F, 0x99, 0x1B };
DHT dht(DHTPIN, DHTTYPE);
SDL_Weather_80422 weatherStation(PIN_ANEMOMETER, PIN_CHUV, 0, 1, PIN_VANE, SDL_MODE_INTERNAL_AD);

void setup() {
   Serial.begin(9600);   
   pinMode(PIN_ANEMOMETER, INPUT);
   pinMode(PIN_SOLARCELL, INPUT);    
   digitalWrite(PIN_ANEMOMETER, HIGH);   
   digitalWrite(PIN_CHUV, HIGH);   
   pinMode(PIN_VANE,INPUT);   
   analogReference(EXTERNAL);   
   attachInterrupt(1,interruptChuv,FALLING);     
   bmp180.begin();       
   weatherStation.setWindMode(SDL_MODE_DELAY, 1);   
   dht.begin();
     
    if(!Ethernet.begin(mac)){
      Serial.println("Falha DHCP");
      Ethernet.begin(mac, ip);
    }    
  else{
    Serial.println("Sucesso DHCP");
    }
        
    Ethernet.begin(mac, ip, ns, gateway, subnet);     
}

void loop() {  
  
  // put your main code here, to run repeatedly:
  for(int i=0;i<16;i++){
    dir[i]=0;    
    }
    
  bmpRead();  
  dhtRead();
  calcWindDir();
  readSolarIrradiation();
     for(int j = 0; j<16; j++){
      Serial.print(dir[j]);
    }
    
    Serial.print("Umidade");
    Serial.println(umid);
    Serial.print("velocidade do vento:");
    Serial.println(weatherStation.current_wind_speed());
    Serial.print("pulso de chuva");
    Serial.println(pchuv);
    Serial.print("temperatura:");
    Serial.println(temp);    
    Serial.print("pressao");
    Serial.println(pres);
    Serial.print("rajada de vento");
    Serial.println(weatherStation.get_wind_gust());
    Serial.print("Irradiação solar:");
    //Serial.println(converteIrradicao);
    delay(1000);
   sendData();
}

void bmpRead(){
  temp=bmp180.readTemperature();
  pres=bmp180.readPressure()/100;
  }

void dhtRead(){  
   umid=dht.readHumidity();
  }  

void calcWindDir() {
   for(int i=0;i<60;i++){
      dir[(int)(weatherStation.current_wind_direction()/22.5)]++;
      delay(1000);
    }
}
String converteDouble(double valor){
  String resp = (String)(int)valor;
  String aux = (String)(int)(valor*100-(int)valor*100);
  resp = resp+"."+aux;
  return resp;  
}

void readSolarIrradiation() {
  irradSolar = analogRead(PIN_SOLARCELL);
}

String converteIrradiacao(){
  String temp;
 //IMPLEMENTAR A EQUAÇÂO DE CONVERSÂO

 temp = (String) irradSolar;
 return temp;
}
void sendData(){
  if(client.connect(server, 8080)) {
        Serial.print("conectou");      
        String vventc= converteDouble(weatherStation.current_wind_speed());
        String umidc = converteDouble(umid);      
        String tempc = converteDouble(temp);
        String presc = converteDouble(pres); 
        String rajventc = converteDouble(weatherStation.get_wind_gust());
        String irradicaoSolar = "11"; //converteIrradicao();
             
        String param="GET /SensoresNaAgricultura/entrada?tipo=ES&umid="+umidc+"&vvent="+vventc+"&pchuv="+(String)pchuv+"&temp="+tempc+"&pres="+presc+"&rajvent="+rajventc;
        
        for(int i=0;i<16;i++){
        param+="&dir"+(String)i+"="+(String)dir[i];
        }       
      client.println(param);
      Serial.println(param);  
      String host="Host: "+(String)server;
      client.println(host);
      Serial.println(host);
      client.println("Connection: close");
      client.println();
      if (client.available()) {
          Serial.println("entrou");
          char c = client.read();
          Serial.print(c);
          pchuv = 0;
    } 
  }
  else{
       Serial.print("falhou");
    }
    client.stop();
  }
  void interruptChuv(){
    pchuv++;
    }   
