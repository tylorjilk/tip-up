/*
  ESP WiFi Arming Code Rev 01
  Created by Tylor Jilk, February 2018
  A part of the IREC Avionics Team of Stanford SSI

  Once the ESP is running, connect to its Wi-Fi network
  and go to one of 3 websites to do exactly what they
  sound like: ipaddress/arm, /disarm, or /status

  The GPIO pin which is controlled by these commands
  is called 'onpin'.
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>

const int shim = 13; // controls the power to the pi
const int pi = 12; // signals to the pi to do an orderly shutdown
const char AP_SSID[] = "Payload";
const char STA_SSID[] = "Skybass";
const char PSK[] = "redshift";


const int checkTime = 2000; //Interval to check for skybass in ms
const int onpin = 5; // IO5 on the Esp8266 WROOM 02

String armed = "Armed";
String disarmed = "Disarmed";
String invalid = "Invalid Request";

WiFiServer server(80);

uint32_t scanTimer = millis();

void setup()
{
  Serial.begin(115200);
  pinMode(shim, OUTPUT);
  pinMode(pi, OUTPUT);
  digitalWrite(shim, LOW); // start out disarmed
  digitalWrite(pi, HIGH);  

  IPAddress ip_s(192, 168, 4, 2);
  IPAddress ip_a(192, 168, 4, 3);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.mode(WIFI_AP_STA);
  WiFi.config(ip_s, ip_s, ip_s, subnet);
  WiFi.softAPConfig(ip_a, ip_a, subnet);
  WiFi.softAP(AP_SSID, PSK);
  server.begin();
  WiFi.begin(STA_SSID,PSK);
}

void handleManualArming()
{
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println("Recvd Request: " + req); //DBG
  client.flush();
  String resp = "";

  if (req.indexOf("/disarm") != -1)
  {
    digitalWrite(pi,0);
    delay(2000);
    digitalWrite(shim,0);
    HTTPRespond(client, disarmed);
  }
  else if (req.indexOf("/arm") != -1)
  {
    digitalWrite(shim, 1);
    digitalWrite(pi, 1);
    HTTPRespond(client, armed);
  }
  else if (req.indexOf("/status") != -1)
  {
    if (digitalRead(shim))
    {
      HTTPRespond(client, armed);
    }
    else
    {
      HTTPRespond(client, disarmed);
    }
  }
  else
  {
    HTTPRespond(client, invalid);
  }
  
}

void checkConnection()
{
  if(WiFi.status()!=WL_CONNECTED)
  {
    WiFi.begin(STA_SSID,PSK);
  }
}

void loop()
{
  handleManualArming();
  checkConnection();
}

void HTTPRespond(WiFiClient client, String message)
{
  //Prepare the response. Start with the common header:
  String resp = "HTTP/1.1 200 OK\r\n";
  resp += "Content-Type: text/html\r\n\r\n";
  resp += "<!DOCTYPE HTML>\r\n<html>\r\n";
  resp += "<head><style>p{text-align:center;font-size:24px;font-family:helvetica;padding:30px;border:1px solid black;background-color:powderblue}</style></head><body>";
  resp += message;
  resp += "</body></html>\n";

  // Send the response to the client
  client.print(resp);
  delay(1);
  Serial.println("Client disconnected"); //DBG

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}
