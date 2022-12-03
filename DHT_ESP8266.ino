#include "DHTesp.h"
#include <ESP8266WiFi.h>
#include <time.h>
#include "CronAlarms.h"
const char* ssid = ""; 
const char* password = ""; 
const char* getHost = ""; 
const int httpGetPort = 80;
String getReceiverURL = "/temp.php";
int hum;
int temp;
DHTesp dht;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  while (!Serial);
    Serial.println("Starting setup...");
    Cron.create("0 */5 * * * *", Repeats, false); // repeat every 5 minutes
    Serial.println("Ending setup...");  
  Serial.println('\n');
  WiFi.hostname("ESPboard-counter");
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);
    Cron.delay(500);
     digitalWrite(LED_BUILTIN, HIGH);
    Cron.delay(500);     
    Serial.print(++i); Serial.print(' ');
  }
  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP()); // Send the IP address of the ESP8266 to the computer
  // Get time from a server
  configTime(2 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // 2 = timezone
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    Cron.delay(500);
  }
  Serial.println("");
  dht.setup(5, DHTesp::DHT22); 
}

void loop(){
    time_t now = time(nullptr);
  Cron.delay(1000);
  delay(dht.getMinimumSamplingPeriod());

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  hum = humidity * 10;
  temp = temperature * 10;
  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("%");
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("°C");
  Serial.print("\t\t");
  Serial.print(dht.toFahrenheit(temperature), 1);
  Serial.print("\t\t");
  Serial.print(dht.computeHeatIndex(temperature, humidity, false), 1);
  Serial.print("\t\t");
  Serial.println(dht.computeHeatIndex(dht.toFahrenheit(temperature), humidity, true), 1);
  Cron.delay(1000);
}

void Repeats() {
  digitalWrite(LED_BUILTIN, LOW);
  postData();
}

void postData() {
	WiFiClient clientGet;
	char sValue[255];
	sprintf(sValue, "%d:%d", hum, temp);
	Serial.print("waardes zijn: ");
	Serial.println(sValue);
	String getReceiverURLtemp = getReceiverURL + "?data=" + sValue;
	Serial.println("-------------------------------");
	Serial.print(">>> Connecting to host: ");
	Serial.println(getHost);
	if (!clientGet.connect(getHost, httpGetPort)) {
		Serial.print("Connection failed: ");
		Serial.print(getHost);
	} else {
		clientGet.println("GET " + getReceiverURLtemp + " HTTP/1.1");
		clientGet.print("Host: ");
		clientGet.println(getHost);
		clientGet.println("User-Agent: ESP8266/1.0");
		clientGet.println("Authorization: Basic c21hcnRtZXRlcjprbm9lcGll=");
		clientGet.println("Connection: close\r\n\r\n");
		unsigned long timeoutP = millis();
		while (clientGet.available() == 0) {
			if (millis() - timeoutP > 10000) {
				Serial.print(">>> Client Timeout: ");
				Serial.println(getHost);
				clientGet.stop();
				return;
			}
		}
		while(clientGet.available()){
			String retLine = clientGet.readStringUntil('\r');
			Serial.print(">>> Host returned: ");
			Serial.println(retLine);
			if (retLine == "HTTP/1.1 200 OK") {
				Serial.println(">>> Communication successful");
			} else {
				Serial.println(">>> Communication failed!!!");
			}
			break;
		}
	}
	Serial.print(">>> Closing host: ");
	Serial.println(getHost);
  digitalWrite(LED_BUILTIN, HIGH);
	clientGet.stop();
}
