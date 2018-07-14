/*
  WiFiTelnetToSerial - Example Transparent UART to Telnet Server for esp8266

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WiFi library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX


//how many clients should be able to telnet to this ESP8266
#define MAX_SRV_CLIENTS 5
const char* ssid = "I-GEOSCAN123";
const char* password = "zzzzzzzz";

WiFiServer server(23);
WiFiClient serverClients[MAX_SRV_CLIENTS];

void setup() {
  delay(2000);
  Serial.begin(9600);
  WiFi.mode(WIFI_AP);
  //  Serial.print("wifi status = ");
  //  Serial.println(WiFi.status());
  WiFi.softAP("think_13", "");
  //  WiFi.begin(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  mySerial.begin(9600);

  /* station mode needed
    Serial.print("\nConnecting to "); Serial.println(ssid);
    uint8_t i = 0;
    while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
    if(i == 21){
    Serial.print("Could not connect to"); Serial.println(ssid);
    while(1) delay(500);
    }
  */
  //start UART and the server
  //  Serial.begin(9600);
  server.begin();
  server.setNoDelay(true);

  Serial.print("Ready! Use 'telnet ");
  //  Serial.print(WiFi.localIP());
  //  Serial.print(myIP);
  //  Serial.println(" 23' to connect");
  pinMode(D5, OUTPUT);

}

void loop() {

  uint8_t i;
  //check if there are any new clients
  if (server.hasClient()) {
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()) {
        if (serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        //        Serial.print("New client: "); Serial.print(i);
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  //check clients for data
  for (i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      if (serverClients[i].available()) {
        //get data from the telnet client and push it to the UART
        char a[10];
        int count = 0;
        while (serverClients[i].available()) {
          char receive_data = serverClients[i].read();
          Serial.write(receive_data);
          mySerial.write(receive_data);
          a[count++] = receive_data;
          if(receive_data == '1'){
            digitalWrite(D5, HIGH);
          }
          if(receive_data == '2'){
            digitalWrite(D5, LOW);
          }
        }
        for (i = 0; i < MAX_SRV_CLIENTS; i++) {
          if (serverClients[i] && serverClients[i].connected()) {
            serverClients[i].write(a, 10);
            delay(1);

          }
        }
      }
    }
  }
  //check UART for data
  if (Serial.available()) {
    size_t len = Serial.available();
    uint8_t sbuf[len];
    Serial.readBytes(sbuf, len);
    //    Serial.println(sbuf[0]);

    //push UART data to all connected telnet clients
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (serverClients[i] && serverClients[i].connected()) {
        serverClients[i].write(sbuf, len);
        delay(1);

      }
    }
  }
}
