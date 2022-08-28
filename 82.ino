#if defined(ESP32)
//Librerias ESP32
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
WiFiMulti wifiMulti;

#elif defined(ESP8266)
//Libreria para ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <LEAmDNS.h>
#include <LEAmDNS_lwIPdefs.h>
#include <LEAmDNS_Priv.h>
#include <WiFiClient.h>
ESP8266WiFiMulti wifiMulti;

#endif

#include "data.h"

int pinLed = D2;
boolean estado = false;

const uint32_t TiempoEsperaWiFi = 5000;
WiFiServer servidor(80);

unsigned long tactual = 0;
unsigned long tanterior = 0;
const long tcancelacion = 500;

void setup() {
  Serial.begin(9600);
  pinMode(pinLed, OUTPUT);
  Serial.println("\nInciando multi WiFi");
  
  wifiMulti.addAP(ssid2, password2);

  WiFi.mode(WIFI_STA);
  Serial.print("Conectando a WiFi ..");
  while (wifiMulti.run(TiempoEsperaWiFi) != WL_CONNECTED){
    Serial.print(".");
  }
  Serial.println(".. Conectado");
  Serial.println("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.println("ID: ");
  Serial.println(WiFi.localIP());
   
  if(!MDNS.begin("codornices")){
    Serial.println("Error en la configuracion DNS!");
    while(1){
      delay(1000);
    }
  }
  Serial.println("mDNS configurado");
  Serial.println("NOMBRE: codornices.local");
  
  servidor.begin();  
  MDNS.addService("http", "tcp", 80);
  
}

void loop() {
#if defined(ESP8266)
  MDNS.update();
#endif

  WiFiClient cliente = servidor.available();

  if(cliente){
    Serial.println("Nuevo Cliente");
    tactual = millis();
    tanterior = tactual;
    String lineaActual = "";

    while (cliente.connected() && tactual - tanterior <= tcancelacion){
      if(cliente.available()){
        tactual = millis();
        char letra = cliente.read();
        if(letra == '\n'){
          if(lineaActual.length() == 0){
            digitalWrite(pinLed, estado);
            ResponderCliente(cliente);
            break;
          }else{
            Serial.println(lineaActual);
            VerificarMensaje(lineaActual);
            lineaActual = "";
          }
        }else if (letra != '\r'){
          lineaActual += letra;
        }
      }
    }
    

    cliente.stop();
    Serial.println("Cliente desconectado");    
  }
  
}

void VerificarMensaje(String mensaje){
  if(mensaje.indexOf("GET /encender") >= 0){
    estado = HIGH;
  }else if (mensaje.indexOf("GET /apagar") >= 0){
    estado = LOW;
  }
}


void ResponderCliente(WiFiClient & cliente){
  cliente.print(Pagina);
  cliente.print("Hola ");
  cliente.print(cliente.remoteIP());
  cliente.print("<br>Estado del led: ");
  cliente.print(estado ? "Encendida" : "Apagada");
  cliente.print("<br>Cambia Led: ");
  cliente.print("<a href = '/");
  cliente.print(estado ? "apagar" : "encender");
  cliente.print("'>Cambiar </a><br>");
  cliente.print("</html>");
}
