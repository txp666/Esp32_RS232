#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char *ssid = "666";          // Enter your WiFi name
const char *password = "12345678"; // Enter WiFi password
const char *mqtt_broker = "192.168.31.110";
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{

  Serial.begin(9600);
  Serial1.begin(19200);


  // connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");
  // connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  // client.setCallback(callback);
  while (!client.connected())
  {
    Serial.println("Connecting to public emqx mqtt broker.....");
    if (client.connect("esp8266-client"))
    {
      Serial.println("Public emqx mqtt broker connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}
unsigned char SendComd[1] = {0X20};
unsigned char ReceiveDate[15] = {};
char BCD_ReceiveDate[50];
void loop()
{

  Serial1.write(SendComd, 1);
  delay(20);
  if (Serial1.available() > 0)
  {
    Serial1.readBytes(ReceiveDate, 15);
  }
  for (int i = 0; i < 15 * 2; i++)
  {
    BCD_ReceiveDate[i] = ((ReceiveDate[i >> 1] >> ((!(i & 1)) << 2)) & 0xf) + '0';

    if (BCD_ReceiveDate[i] > '9')
    {
      BCD_ReceiveDate[i] = '0';
    }
  }

  Serial.print("MyTorque: ");
  Serial.println(BCD_ReceiveDate);
  client.publish("motor", BCD_ReceiveDate, 15);
  delay(100);

  client.loop();
}