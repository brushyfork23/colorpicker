/*
Feather Huzzah ESP8622 WiFi access point
which receives a color from the color picker
then writes it out over serial.
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

/* Set these to your desired credentials. */
const char *ssid = "ssid";
const char *password = "password";

ESP8266WebServer server(80);

uint32_t code;

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot() {
  server.send(200, "text/html", "<h1>ColorHub</h1>");
}

void handleColor() {
  Serial.println("Handling /color");

  if (server.args()) {
    code = server.arg(0);

    Serial.print("setting code to: ");
    Serial.println(code);

    server.send(200, "text/html", "Setting code");
  } else {
    server.send(400, "text/html", "No code received");
  }
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
 server.on("/color", handleColor);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

