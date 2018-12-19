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

const char WiFiAPPSK[] = "heytheresexy";
const int onpin = 2; // IO5 on the Esp8266 WROOM 02

WiFiServer server(80);

void setup() {
    Serial.begin(115200);
    pinMode(onpin, OUTPUT);
    digitalWrite(onpin, LOW);

    WiFi.mode(WIFI_AP);

    // Do a little work to get a unique-ish name. Append the
    // last two bytes of the MAC (HEX'd) to "Thing-":
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);
    String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                    String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
    macID.toUpperCase();
    String AP_NameString = "Main AV " + macID;

    char AP_NameChar[AP_NameString.length() + 1];
    memset(AP_NameChar, 0, AP_NameString.length() + 1);

    for (int i=0; i<AP_NameString.length(); i++)
        AP_NameChar[i] = AP_NameString.charAt(i);

    WiFi.softAP(AP_NameChar, WiFiAPPSK);

    server.begin();
}

void loop() {

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  // Match the request
  int val = -1; // We'll use 'val' to keep track of both the
                // request type (read/set) and value if set.
  if (req.indexOf("/off") != -1)
    val = 0; // Will write LED low
  else if (req.indexOf("/on") != -1)
    val = 1; // Will write LED high
  else if (req.indexOf("/status") != -1)
    val = -2; // Will print pin reads
  // Otherwise request will be invalid. We'll say as much in HTML

  // Set GPIO5 according to the request
  if (val >= 0)
    digitalWrite(onpin, val);

  client.flush();

  // Prepare the response. Start with the common header:
  String s = "";
  // String s = "HTTP/1.1 200 OK\r\n";
 //  s += "Content-Type: text/html\r\n\r\n";
 // s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  //s += "<head><style>p{text-align:center;font-size:24px;font-family:helvetica;padding:30px;border:1px solid black;background-color:powderblue}</style></head><body>";

  // If we're setting the LED, print out a message saying we did
  if (val == 0)
  {
    s += "        off";
  } 
  else if (val == 1)
  {
    s += "        on";
  } 
  else if (val == -2)
  {
    // s += "<br>"; // Go to the next line.
    s += "        Status: ";
    if(digitalRead(onpin))
    {
      s += "on";
    } 
    else
    {
      s += "off";
    }
  } 
  else
  {
    s += "        Invalid Request. Try /disarm, /arm, or /status.";
  }

  //s += "</body></html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}
