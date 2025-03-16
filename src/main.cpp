/*
 * ESP32 Wi-Fi használat a Projekthez
 * Hallgatói verzió - 2024.05.23
 * HTML/CSS/JS fejlesztéssel
 */

 #include <Arduino.h>
 #include <WiFi.h>
 #include <ArduinoOTA.h>
 #include <WebServer.h>
 #include <ESPmDNS.h>
 
 // TODO: Jelszavakat később kiszedni a kódbál!!!
 const char* ssid = "Telekom-760933";
 const char* jelszo = "87630692866353068315";
 
 WebServer szerver(80);
 
 // Változók a kapcsolódási állapot követéséhez
 int kapcsolodasi_probalkozas = 0;
 bool wifi_connected = false;
 
 // Mérési adatok
 float terheles = 0.0;
 float szog = 90.0;
 unsigned long utolso_adatkuldes = 0;
 
 String generaldHTML() {
   String html = R"rawliteral(
   <!DOCTYPE html>
   <html lang="hu">
   <head>
     <meta charset="UTF-8">
     <meta name="viewport" content="width=device-width, initial-scale=1.0">
     <title>Térd Monitor</title>
     <style>
       :root {
         --primary: #2c3e50;
         --secondary: #3498db;
         --background: #f8f9fa;
         --text: #2c3e50;
         --success: #28a745;
         --danger: #dc3545;
       }
 
       body {
         font-family: 'Segoe UI', system-ui;
         margin: 0;
         padding: 20px;
         background: var(--background);
         color: var(--text);
       }
 
       .container {
         max-width: 800px;
         margin: 0 auto;
       }
 
       .header {
         background: var(--primary);
         color: white;
         padding: 2rem;
         border-radius: 12px;
         margin-bottom: 2rem;
         box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
       }
 
       .dashboard {
         display: grid;
         grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
         gap: 1.5rem;
       }
 
       .card {
         background: white;
         padding: 1.5rem;
         border-radius: 12px;
         box-shadow: 0 2px 4px rgba(0, 0, 0, 0.05);
         transition: transform 0.2s;
       }
 
       .card:hover {
         transform: translateY(-2px);
       }
 
       .metric-value {
         font-size: 2.5rem;
         font-weight: 600;
         color: var(--primary);
         margin: 1rem 0;
       }
 
       .metric-unit {
         font-size: 1.2rem;
         color: #6c757d;
       }
 
       .status-indicator {
         display: inline-block;
         width: 12px;
         height: 12px;
         border-radius: 50%;
         margin-right: 0.5rem;
       }
 
       .connected { background: var(--success); }
       .disconnected { background: var(--danger); }
 
       button {
         background: var(--secondary);
         color: white;
         border: none;
         padding: 0.8rem 1.5rem;
         border-radius: 8px;
         cursor: pointer;
         font-size: 1rem;
         transition: opacity 0.2s;
         width: 100%;
         margin-top: 1rem;
       }
 
       button:hover {
         opacity: 0.9;
       }
 
       @media (max-width: 600px) {
         .header {
           padding: 1.5rem;
         }
         
         .metric-value {
           font-size: 2rem;
         }
       }
     </style>
   </head>
   <body>
     <div class="container">
       <div class="header">
         <h1>Térd Monitor Rendszer</h1>
         <p>IP cím: <span id="ip-cim">%IP_CIM%</span></p>
         <p>Kapcsolat állapota: 
           <span class="status-indicator" id="kapcsolat-statusz"></span>
           <span id="kapcsolat-szoveg"></span>
         </p>
       </div>
 
       <div class="dashboard">
         <div class="card">
           <h2>Terhelés</h2>
           <div class="metric-value">
             <span id="terheles">0</span>
             <span class="metric-unit">kg</span>
           </div>
         </div>
 
         <div class="card">
           <h2>Ízületszög</h2>
           <div class="metric-value">
             <span id="szog">90</span>
             <span class="metric-unit">°</span>
           </div>
         </div>
       </div>
 
       <button onclick="handleRestart()">Eszköz újraindítása</button>
     </div>
 
     <script>
       function updateMetrics() {
         fetch('/adatok')
           .then(response => {
             if(!response.ok) throw new Error('Hálózati hiba');
             return response.json();
           })
           .then(data => {
             // Adatok frissítése
             document.getElementById('terheles').textContent = data.terheles.toFixed(1);
             document.getElementById('szog').textContent = data.szog.toFixed(1);
             
             // Kapcsolat állapota
             const statusElem = document.getElementById('kapcsolat-statusz');
             const statusText = document.getElementById('kapcsolat-szoveg');
             if(data.kapcsolat) {
               statusElem.className = 'status-indicator connected';
               statusText.textContent = 'Online';
             } else {
               statusElem.className = 'status-indicator disconnected';
               statusText.textContent = 'Offline';
             }
           })
           .catch(error => {
             console.error('Hiba történt:', error);
           })
           .finally(() => {
             setTimeout(updateMetrics, 500);
           });
       }
 
       function handleRestart() {
         if(confirm('Biztos szeretné újraindítani az eszközt?')) {
           fetch('/restart')
             .then(() => {
               setTimeout(() => {
                 window.location.reload();
               }, 2000);
             });
         }
       }
 
       // Indítsd el a frissítési ciklust
       updateMetrics();
     </script>
   </body>
   </html>
   )rawliteral";
 
   html.replace("%IP_CIM%", WiFi.localIP().toString());
   return html;
 }
 
 void handleAdatok() {
   String json = "{";
   json += "\"terheles\":" + String(terheles) + ",";
   json += "\"szog\":" + String(szog) + ",";
   json += "\"kapcsolat\":" + String(wifi_connected ? "true" : "false");
   json += "}";
   szerver.send(200, "application/json", json);
 }
 
 // A többi függvény változatlan marad...
 
 void handleRoot() {
   szerver.send(200, "text/html", generaldHTML());
 }
 
 void handleRestart() {
   szerver.send(200, "text/plain", "Újraindítás...");
   delay(1000);
   ESP.restart();
 }
 
 void wifiCsatlakozas() {
   Serial.println("WiFi csatlakozás...");
   WiFi.begin(ssid, jelszo);
   
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
   }
 }
 
 void setup() {
   Serial.begin(115200);
   while(!Serial);
 
   WiFi.mode(WIFI_STA);
   wifiCsatlakozas();
   
   if(!wifi_connected) {
     Serial.println("Újrapróbálkozás...");
     wifiCsatlakozas();
   }
 
   ArduinoOTA.setHostname("TerdMonitor"); 
   ArduinoOTA.setPassword("admin");     
   ArduinoOTA.begin();
 
   szerver.on("/", handleRoot);
   szerver.on("/adatok", handleAdatok);
   szerver.on("/restart", handleRestart);
   szerver.begin();
   
   Serial.println("Szerver elindult");
 }
 
 void loop() {
   szerver.handleClient();
   ArduinoOTA.handle();
   
   if(millis() - utolso_adatkuldes > 100) {
     terheles = random(0, 100) / 10.0;
     szog = 90 + (random(0, 60) - 30);
     utolso_adatkuldes = millis();
   }
 
   static unsigned long utolso_ellenorzes = 0;
   if(millis() - utolso_ellenorzes > 10000) {
     if(WiFi.status() != WL_CONNECTED) {
       Serial.println("WiFi kapcsolat megszakadt!");
       wifi_connected = false;
       wifiCsatlakozas();
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