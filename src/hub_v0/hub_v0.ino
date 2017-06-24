/*
Feather Huzzah ESP8622 WiFi access point
which receives a color from the color picker
then writes it out over serial.
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

/* Set these to your desired credentials. */
const char *ssid = "ColorHub";
const char *password = "password";

IPAddress local_IP(192,168,4,19);
IPAddress gateway(192,168,4,20);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot() {
  server.send(200, "text/html", "<h1>Look!  I'm a web page running on that tiny device over there!</h1>");
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(ssid, password) ? "Ready" : "Failed!");

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}


