#include <ESP8266WiFi.h>
 
// definizione costanti
const char WiFiSSID[] = "ssid-xxx";
const char WiFiPSK[] = "password";
const int RELE1_PIN = 16; // D0 - GPI16
const int RELE2_PIN = 5; // D1 - GPIO5
const int DELAY_RELE = 700;  // ritardo in millisecondi
// IP statico
IPAddress ip(192, 168, 1, 50);
IPAddress gateway(192,168,1,254);
IPAddress subnet(255,255,255,0);
 
// inizializzazione server
WiFiServer server(80);
 
void initWiFi() {
  // tento di connetermi alla WI-FI
  Serial.println();
  Serial.println("Attempting to connect to: " + String(WiFiSSID) + " ...");
  //attendo la disponibilità del wifi
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  // Mi connetto alla WIFI ( WPA/WPA2 )
  WiFi.begin(WiFiSSID, WiFiPSK);
  // setto l'IP statico
  WiFi.config(ip,gateway,subnet);
  Serial.println();
  Serial.print("WiFi connected IP:");
  Serial.print(WiFi.localIP());
}
 
void initServer(){
  // avvio il server
  server.begin();
  // visualizzo l'indirizzo IP
  Serial.println();
  Serial.print("Use this URL to connect: http://");
  Serial.print(WiFi.localIP());
  Serial.print("/rele/[1|2]");
  Serial.println();
}
 
void digitalWritePulse(int pin, int delayMilliseconds){
    digitalWrite(RELE1_PIN, LOW);
    delay(delayMilliseconds);
    digitalWrite(RELE1_PIN, HIGH);
}
 
void sendResponse(WiFiClient client, int result){
  // invio la risposta
  client.flush();
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); // questo è indispensabile
  if(result == -1){
    client.print("ERROR");
    Serial.println("ERROR");
  } else {
    client.print(result);
    Serial.println(result);
  }
}
 
// inizio
void setup() {
  Serial.begin(9600);
  delay(10);
  // imposto le uscite
  pinMode(RELE1_PIN, OUTPUT);
  digitalWrite(RELE1_PIN, HIGH);
  pinMode(RELE2_PIN, OUTPUT);
  digitalWrite(RELE2_PIN, HIGH);
 
  initWiFi();
  initServer();
}
 
void loop() {
  // aspetto che un client si connetta
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  // attendo che invii qualche dato
  while(!client.available()){
    delay(1);
  }
  // leggo la prima linea della richiesta (attenzione è CASE SENSITIVE)
  String request = client.readStringUntil('\r');
  Serial.println(request);
 
  // routing gestione url
  // verifico la url e attivo il relè per N secondi
  int releNumber = -1;
  if (request.indexOf("GET /rele/1 ") != -1) {
    digitalWritePulse(RELE1_PIN, DELAY_RELE);
    releNumber= 1;
  } else if (request.indexOf("GET /rele/2 ") != -1) {
    digitalWritePulse(RELE2_PIN, DELAY_RELE);
    releNumber = 2;
  } else if (request.indexOf("GET /rele/1/on ") != -1) {
    digitalWrite(RELE1_PIN, LOW);
    releNumber = 1;
  } else if (request.indexOf("GET /rele/1/off ") != -1) {
    digitalWrite(RELE1_PIN, HIGH);
    releNumber = 0;
  }  
   
  sendResponse(client, releNumber);
}