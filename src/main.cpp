/*
 * ESP32 Wi-Fi használat a  Projekthez
 */

 #include <Arduino.h>
 #include <WiFi.h>
 #include <ArduinoOTA.h>
 #include <WebServer.h>
 #include <ESPmDNS.h>
 #include <ArduinoOTA.h>
 
 // TODO: Jelszavakat később kiszedni a kódbál!!!
 const char* ssid = "Telekom-760933";
 const char* jelszo = "87630692866353068315";
 
 WebServer szerver(80);  // Port beállítva kézzel, valamiért az 8080 nem működött
 
 // Változók a kapcsolódási állapot követéséhez
 int kapcsolodasi_probalkozas = 0;
 bool wifi_connected = false;
 
 void handleRoot() {
   // Csicska HTML, 
   String html = "<!DOCTYPE html><html><head>";
   html += "<meta charset='UTF-8'>";  // 
   html += "<title>ESP32 Webszerver</title>";
   html += "<style>body{font-family:Arial;text-align:center}</style>";
   html += "</head><body>";
   html += "<h1>Szevasztok!</h1>";
   html += "<p>ESP32 szerver működik</p>";
   html += "<p>IP cím: " + WiFi.localIP().toString() + "</p>";
   html += "<button onclick=\"location.href='/restart'\">Újraindítás</button>";
   html += "</body></html>";
   
   szerver.send(200, "text/html", html);
 }
 
 void handleRestart() {
   szerver.send(200, "text/plain", "Újraindítás...");
   delay(1000);
   ESP.restart();  // maybe this will work idk
 }
 
 void wifiCsatlakozas() {
   Serial.println("WiFi csatlakozás...");
   WiFi.begin(ssid, jelszo);
   
   // 15 próbálkozás a csatlakozásra
   while (WiFi.status() != WL_CONNECTED && kapcsolodasi_probalkozas < 15) {
     delay(500);
     Serial.print(".");
     kapcsolodasi_probalkozas++;
   }
   
   if(WiFi.status() == WL_CONNECTED) {
     Serial.println("\nCsatlakozva!");
     Serial.print("IP cím: ");
     Serial.println(WiFi.localIP());
     wifi_connected = true;
   } else {
     Serial.println("\nHiba! Nem sikerült csatlakozni");
     // Itt valami hibakezelés kellene...
   }
 }
 
 void setup() {
   Serial.begin(115200);
   while(!Serial); // Várakozás a soros port csatlakozására
   
   // WiFi rész - 2. variáció
   WiFi.mode(WIFI_STA);
   wifiCsatlakozas();
   
   // Ha nem sikerült akkor most fog
   if(!wifi_connected) {
     Serial.println("Újrapróbálkozás...");
     wifiCsatlakozas();
   }
 
   // ArduinoOTA inicializálása
   ArduinoOTA.setHostname("ESP32-OTAN"); 
   ArduinoOTA.setPassword("admin");     
   ArduinoOTA.begin();
   Serial.println("OTA frissítés kész!");
 
   // Webszerver beállítások
   szerver.on("/", handleRoot);
   szerver.on("/restart", handleRestart);
   szerver.begin();
   
   Serial.println("Szerver elindult... talán...");
 }
 
 void loop() {
   szerver.handleClient();
   ArduinoOTA.handle();  // OTA frissítések kezelése
   
   // Időnként kéne a kapcsolatot ellenőrizni 
   static unsigned long utolso_ellenorzes = 0;
   if(millis() - utolso_ellenorzes > 10000) {
     if(WiFi.status() != WL_CONNECTED) {
       Serial.println("WiFi kapcsolat megszakadt!");
       wifi_connected = false;
       // Itt kéne valami újracsatlakozási logika...
       wifiCsatlakozas();  // Újracsatlakozás
     }
     utolso_ellenorzes = millis();
   }
 }
 
 /*
  * Ismert problémák:
  * 1. Néha nem találja az IP címet elsőre
  * 2. Újracsatlakozás nem mindig működik
  * 
  * Hibakeresési tippek:
  * - router restart
  * - COM port (ugyanoda kéne dugni)
  */