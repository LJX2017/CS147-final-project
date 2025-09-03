


#include <Arduino.h>
#include <Servo.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "Wire.h"
#include "DHT20.h"
#include <ArduinoJson.h>
#include <string>


#define soilPower 12
#define soilPin 33
#define photoPin 32
#define SDA  17
#define SCL  15
#define seroePin 25
#define photoThresh 3900 // for 2k resistor
int val = 0;
DHT20 DHT;

Servo myservo;
// Wi-Fi credentials
#define WIFI_SSID "UCInet Mobile Access" // NOTE: Please delete this value before submitting assignment
#define WIFI_PASSWORD "" // NOTE: Please delete this value before submitting assignment
// Azure IoT Hub configuration
#define SAS_TOKEN "SharedAccessSignature sr=butter.azure-devices.net%2Fdevices%2F147esp32&sig=WAZHL%2FKWRBzJK0eG%2FoI%2BvQcDPbU6yYnSySIBqrEaFBw%3D&se=1760714122"
// Root CA certificate for Azure IoT Hub
const char* root_ca = R"(-----BEGIN CERTIFICATE-----
MIIEtjCCA56gAwIBAgIQCv1eRG9c89YADp5Gwibf9jANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
MjAeFw0yMjA0MjgwMDAwMDBaFw0zMjA0MjcyMzU5NTlaMEcxCzAJBgNVBAYTAlVT
MR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xGDAWBgNVBAMTD01TRlQg
UlMyNTYgQ0EtMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMiJV34o
eVNHI0mZGh1Rj9mdde3zSY7IhQNqAmRaTzOeRye8QsfhYFXSiMW25JddlcqaqGJ9
GEMcJPWBIBIEdNVYl1bB5KQOl+3m68p59Pu7npC74lJRY8F+p8PLKZAJjSkDD9Ex
mjHBlPcRrasgflPom3D0XB++nB1y+WLn+cB7DWLoj6qZSUDyWwnEDkkjfKee6ybx
SAXq7oORPe9o2BKfgi7dTKlOd7eKhotw96yIgMx7yigE3Q3ARS8m+BOFZ/mx150g
dKFfMcDNvSkCpxjVWnk//icrrmmEsn2xJbEuDCvtoSNvGIuCXxqhTM352HGfO2JK
AF/Kjf5OrPn2QpECAwEAAaOCAYIwggF+MBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYD
VR0OBBYEFAyBfpQ5X8d3on8XFnk46DWWjn+UMB8GA1UdIwQYMBaAFE4iVCAYlebj
buYP+vq5Eu0GF485MA4GA1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcD
AQYIKwYBBQUHAwIwdgYIKwYBBQUHAQEEajBoMCQGCCsGAQUFBzABhhhodHRwOi8v
b2NzcC5kaWdpY2VydC5jb20wQAYIKwYBBQUHMAKGNGh0dHA6Ly9jYWNlcnRzLmRp
Z2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RHMi5jcnQwQgYDVR0fBDswOTA3
oDWgM4YxaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0R2xvYmFsUm9v
dEcyLmNybDA9BgNVHSAENjA0MAsGCWCGSAGG/WwCATAHBgVngQwBATAIBgZngQwB
AgEwCAYGZ4EMAQICMAgGBmeBDAECAzANBgkqhkiG9w0BAQsFAAOCAQEAdYWmf+AB
klEQShTbhGPQmH1c9BfnEgUFMJsNpzo9dvRj1Uek+L9WfI3kBQn97oUtf25BQsfc
kIIvTlE3WhA2Cg2yWLTVjH0Ny03dGsqoFYIypnuAwhOWUPHAu++vaUMcPUTUpQCb
eC1h4YW4CCSTYN37D2Q555wxnni0elPj9O0pymWS8gZnsfoKjvoYi/qDPZw1/TSR
penOgI6XjmlmPLBrk4LIw7P7PPg4uXUpCzzeybvARG/NIIkFv1eRYIbDF+bIkZbJ
QFdB9BjjlA4ukAg2YkOyCiB8eXTBi2APaceh3+uBLIgLk8ysy52g2U3gP7Q26Jlg
q/xKzj3O9hFh/g==
-----END CERTIFICATE-----)";

String iothubName = "butter"; //Your hub name (replace if needed)
String deviceName = "147esp32"; //Your device name (replace if needed)
String url = "https://" + iothubName + ".azure-devices.net/devices/" + deviceName + "/messages/events?api-version=2021-04-12";

// Telemetry interval
#define TELEMETRY_INTERVAL 3000 // Send data every 3 seconds

bool is_shaded = false;
uint8_t count = 0;
uint32_t lastTelemetryTime = 0;


int readSoil()
{
    digitalWrite(soilPower, HIGH);//turn D7 "On"
    delay(10);//wait 10 milliseconds 
    val = analogRead(soilPin);//Read the SIG value form sensor 
    digitalWrite(soilPower, LOW);//turn D7 "Off"
    return val;//send current moisture value
}

int readPhoto()
{
    return analogRead(photoPin);
}


void readTempAndHumidity(float &temperature, float &humidity) {
    int err = DHT.read();
    temperature = DHT.getTemperature();
    humidity = DHT.getHumidity();
    switch (err)
    {
      case DHT20_OK:
        Serial.print("OK");
        break;
      case DHT20_ERROR_CHECKSUM:
        Serial.print("Checksum error");
        break;
      case DHT20_ERROR_CONNECT:
        Serial.print("Connect error");
        break;
      case DHT20_MISSING_BYTES:
        Serial.print("Missing bytes");
        break;
      case DHT20_ERROR_BYTES_ALL_ZERO:
        Serial.print("All bytes read zero");
        break;
      case DHT20_ERROR_READ_TIMEOUT:
        Serial.print("Read time out");
        break;
      case DHT20_ERROR_LASTREAD:
        Serial.print("Error read too fast");
        break;
      default:
        Serial.print("Unknown error");
        break;
    }
}

void sendTelemetry(int soilMoisture, int PhotoResistance, float temperature, float humidity) {
        // Create JSON payload
        ArduinoJson::JsonDocument doc;
        doc["temperature"] = temperature;
        doc["humidity"] = humidity;
        doc["soilMoisture"] = soilMoisture;
        doc["photoResistance"] = PhotoResistance;
        char buffer[256];
        serializeJson(doc, buffer, sizeof(buffer));

        // Send telemetry via HTTPS
        WiFiClientSecure client;
        client.setCACert(root_ca); // Set root CA certificate
        HTTPClient http;
        http.begin(client, url);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", SAS_TOKEN);
        int httpCode = http.POST(buffer);
        if (httpCode == 204) { // IoT Hub returns 204 No Content for successful telemetry
            Serial.println("Telemetry sent: " + String(buffer));
        } else {
            Serial.println("Failed to send telemetry. HTTP code: " + String(httpCode));
        }
        http.end();

        lastTelemetryTime = millis();
}


void setup() 
{
    Serial.begin(9600);
    delay(1000);
    Wire.begin(SDA, SCL);
    DHT.begin();
    delay(1000);
    WiFi.mode(WIFI_STA);
    delay(1000);
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);
    myservo.attach(25);

    //Initialize DHT20 here

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  pinMode(soilPower, OUTPUT);//Set D7 as an OUTPUT
  pinMode(photoPin, INPUT);
  digitalWrite(soilPower, LOW);//Set to LOW so no power is flowing through the sensor
}

void loop() 
{
  if (is_shaded) {
    // we don't want to continuously cover the plant.
    myservo.write(180); // lift the shade
    is_shaded = false;
  } else {
    if (readPhoto() > photoThresh) {
      myservo.write(0); // lower the shade
      is_shaded = true;
    }
  }
  Serial.print("Soil Moisture = ");    
  Serial.println(readSoil());
  Serial.print("Photo Resistance = ");
  Serial.println(readPhoto());
  float temperature, humidity;
  readTempAndHumidity(temperature, humidity);
  Serial.print("Temperature = ");
  Serial.println(temperature);
  Serial.print("Humidity = ");
  Serial.println(humidity);
  // Check if it's time to send telemetry
  if (millis() - lastTelemetryTime >= TELEMETRY_INTERVAL) {
    int soilMoisture = readSoil();
    int photoResistance = readPhoto();
    readTempAndHumidity(temperature, humidity);

    // Send telemetry data
    sendTelemetry(soilMoisture, photoResistance, temperature, humidity);
  }
  delay(3000);
}
