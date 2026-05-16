#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "time.h"

// 🔐 WiFi
const char* ssid = "SJIT-Girls-Hostel-Third-Floor";
const char* password = "sjit@tech$g3";

// 🌐 AWS IoT
const char* mqttServer = "av2r6ck2uokdg-ats.iot.us-east-1.amazonaws.com";
const int mqttPort = 8883;
const char* topic = "medicine/status";

// 🌐 Time (India)
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

// 📌 Pins
#define IR_SENSOR 34
#define BUZZER 25

// ⏰ Medicine Time
int medHour[3]   = {11, 14, 21};
int medMinute[3] = {0, 0, 0};

// Flags
bool medicineTaken[3] = {false, false, false};
bool alertDone[3]     = {false, false, false};

// 🔐 ROOT CA
static const char* root_ca = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

// 🔐 DEVICE CERT
static const char* device_cert = R"KEY(
  -----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUWM39Lm2E39/GoxpBrlzBB+ZsM7EwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI2MDQyNTE1MjQz
OVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMa5hkuSAAy3yygiGLWP
msrf11BfYyYcprrm1TsvNUSjORqnhy6OT4wgqRlSUSiurXxViolOkPeOol0yU3hO
NohJH0DVlL2SvTyjfvYvWeIVJSMukPQ+6I/kz4lur/q/ddxz6wZS/ReRfXTJG+wp
WMtbpVCr+i3Hzxgw6WpiRr1oKJkho6M3s5hvfxzRZFbSz1LRkgowkpwUlF8q68MC
QftkkCyj2p836ZkGE08xn3+L0Q8LMDW3+q3ERKwEHNNkF5PVkikx8rN4dK5Y/P6G
ShC21Hsr+Aeink0KZ4SYeyzjal2+MTjBvgS/OGE6P4OQhCmCV6TlmAxTuFwT+5//
ajECAwEAAaNgMF4wHwYDVR0jBBgwFoAUCuC/VJmV3Tet7Ot4ZshitfpIFFIwHQYD
VR0OBBYEFNdf3CjELUPqrJbHvdY+X0mvZt0ZMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBiozee9AWxkAv3JKqRM7W6FOG6
CG9nxA4AOk4C+j1ave8ZmTINVjpN/GRtwWGBLIGBq2FrPvOkurlEPxrX5b3Cfx8A
X3g1nyJ1sbyyER2DOGS5K9Tn3kRETo/rpc1uqqbLt92YXF7ZcfagdcHYtV5IdvZM
Do3aZTPKtUxTBA14zR7oMme87Ng3AmXI9x6m3buKbuzlWSDfSCmkGykln7tZ7xw1
Ofp8Spm7CQKjwedaM2oA3SpuOVQH46kMAuWSxNleQ2wOuFWvwq4imse7NFhTDz8h
Q7iT/8pKIW17ObyMCwX82XI8Mf7f7UHbob7+H27nz3U2SJ8oaDghNi0PRkqA
-----END CERTIFICATE-----
)KEY";

// 🔐 PRIVATE KEY
static const char* private_key = R"KEY(
  -----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEAxrmGS5IADLfLKCIYtY+ayt/XUF9jJhymuubVOy81RKM5GqeH
Lo5PjCCpGVJRKK6tfFWKiU6Q946iXTJTeE42iEkfQNWUvZK9PKN+9i9Z4hUlIy6Q
9D7oj+TPiW6v+r913HPrBlL9F5F9dMkb7ClYy1ulUKv6LcfPGDDpamJGvWgomSGj
ozezmG9/HNFkVtLPUtGSCjCSnBSUXyrrwwJB+2SQLKPanzfpmQYTTzGff4vRDwsw
Nbf6rcRErAQc02QXk9WSKTHys3h0rlj8/oZKELbUeyv4B6KeTQpnhJh7LONqXb4x
OMG+BL84YTo/g5CEKYJXpOWYDFO4XBP7n/9qMQIDAQABAoIBADrw3+IeJaGgCcOX
iVighTrS9VvDTvcI3HLm8CujrtryxocfVTr7vqBDN6RpA+VbGd2cyYA8K0chqQqX
WKaAHDCD758dOgaYNUR9AbUnCjPCLcenkhq/Zd738I5EpDFtBBokz3qKtsJ8xocS
9xMumEWIoxmwSH23m7cDGtgQ2gZnDdj35QikmeQzkwe9QtaTj+wrM8E1DPEkqbqX
qSY576FRlZ1RC9CuZLyqF5xcY7oCNHA1BNZpLoLf/HVFsBql921DB5oY9KIOuPp6
TW0YLyrsOCSYGluRNC6dkNwzRAWlD4f7E1L3cMp5I7XNk1cX6W1gL21Q6MbXNHjt
t2Ks1SECgYEA+X6CJAWCilnamAUwSQecuU7nj0oL8R90D2RDoe/9dJdGcO2g3i6W
wmZ6f6YicU0YLDDC6wTJtlXVHes5o4dvw9zr7FX9HysMv1jFKc8jiYvIAIZK50Qq
wm8B40+fR23e1KmLj50QyWrTF2+rPGVb1uA7Qwx5znT3YpU90lX/1F0CgYEAy+gb
I6EOw2cgThClNUhvw63zjAD0oxdNOGtHU/d8TztGQI34g58Jwe6Toh17mGv0BSLa
Vss7KwekkJj9L9RegNGrFBivMWwWU4R47tLpHBbkTd/bQhOgiAdqvuwWrID4+sih
N2dy2Twm/xxD9DocHvrQv/iVpAiSKDTbX1w5D+UCgYEA3vPLTm9SEVpsei8g0BIq
XnqBeU29UDNAXsTFn3+76iWJAD+J4yKU/JHhZBLNcpxnPo4wkR6NNxCKyxq6PkU4
fjn85gyNjv3EfXXRcyQ06KRuDPH1G6eR4oCamkw71PP9DeOY4LSGHMgGbUoQJn+Z
S3LD4o3Sh8mEjKc7QKwzWs0CgYAMO9TFAni8SIerp1wCImJyPxGJqNUEo/VCGU2i
QAr5RfklDl+GphgTxpic9V/g+TkexcSLyI24OzEoa8VlBTzWRb4Z6pE4Sd/dCumm
iojp4oW4FmI1SLAs+STyZ3/EwcguY7Moe9BsMCu2yp9ZG9NsGGhF3Eg+GzWJrAIt
an8R8QKBgQCJSfYgLV7s95rDO8GNDNcZJbmRYlWtomeatTt0irz+cKkYQK/txdq6
qdMYe7GohCrfMFdWtfZMN638+W8GLBgb2tzxMKCdxVrVJlWEw4smU5BDZrQwKSqB
VNSzc0yI8m7t7QM6obw5cfeeqEkkEZ4zyl0FxpeUdoMRHYlWB5Xz3Q==
-----END RSA PRIVATE KEY-----
)KEY";

WiFiClientSecure net;
PubSubClient client(net);

// ✅ WiFi
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi Connected");
}

// ✅ Time Sync
void syncTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  Serial.print("Syncing time");
  time_t now = time(nullptr);

  while (now < 100000) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  Serial.println("\n✅ Time Synced");
}

// ✅ AWS
void connectAWS() {
  net.setCACert(root_ca);
  net.setCertificate(device_cert);
  net.setPrivateKey(private_key);

  client.setServer(mqttServer, mqttPort);

  while (!client.connected()) {
    Serial.println("Connecting AWS IoT...");

    if (client.connect("ESP32Client")) {
      Serial.println("✅ AWS Connected");
    } else {
      Serial.print("❌ Failed, State: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

// ✅ Publish
void publishMessage(int slot, String status) {
  String payload = "{\"slot\":";
  payload += slot;
  payload += ",\"status\":\"";
  payload += status;
  payload += "\"}";

  Serial.println(payload);
  client.publish(topic, payload.c_str());
}

void setup() {
  Serial.begin(115200);

  pinMode(IR_SENSOR, INPUT);
  pinMode(BUZZER, OUTPUT);

  connectWiFi();
  syncTime();
  connectAWS();
}

void loop() {
  if (!client.connected()) connectAWS();
  client.loop();

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  int h = timeinfo.tm_hour;
  int m = timeinfo.tm_min;

  for (int i = 0; i < 3; i++) {

    if (h == medHour[i] && m == medMinute[i]) {

      Serial.println("⏰ Medicine Time!");
      medicineTaken[i] = false;

      for (int j = 0; j < 30; j++) {

        if (digitalRead(IR_SENSOR) == LOW) {
          Serial.println("✅ Taken");
          publishMessage(i + 1, "taken");
          medicineTaken[i] = true;
          break;
        }

        delay(1000);
      }

      if (!medicineTaken[i] && !alertDone[i]) {

        Serial.println("❌ Missed");
        publishMessage(i + 1, "missed");

        for (int k = 0; k < 10; k++) {
          digitalWrite(BUZZER, HIGH);
          delay(500);
          digitalWrite(BUZZER, LOW);
          delay(500);
        }

        alertDone[i] = true;
      }
    }
  }

  delay(1000);
}