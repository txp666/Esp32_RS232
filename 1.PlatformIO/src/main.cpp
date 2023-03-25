#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
// WiFi网络设置
const char *ssid = "666";
const char *password = "12345678";
const char *mqtt_server="192.168.31.2";
struct SensorData
{
  float torque;
  float speed;
  float power;
};

 float decodeFloat(unsigned char *data)
 {
   // 提取尾数
   float mantissa = ((data[0] & 0xF0) >> 4) * 1000 + (data[0] & 0x0F) * 100 + ((data[1] & 0xF0) >> 4) * 10 + (data[1] & 0x0F);
   mantissa += (((data[2] & 0xF0) >> 4) * 1000 + (data[2] & 0x0F) * 100 + ((data[3] & 0xF0) >> 4) * 10 + (data[3] & 0x0F)) * 0.0001;
   mantissa = mantissa / 1000;
   // 提取阶码
   int8_t exponent = data[4] & 0x3F;
 
   // 提取数字符号
   if (data[4] & 0x80)
   {
     mantissa = -mantissa;
   }
 
   // 提取阶符
   if (data[4] & 0x40)
   {
     exponent = -exponent;
   }
 
   // 计算浮点数
   float result = mantissa * pow(10, exponent);
 
   return result;
 }

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

QueueHandle_t dataQueue;
TaskHandle_t mqttTaskHandle;

void mqttTask(void *pvParameters)
{
  while (true)
  {
    // 从队列中获取数据
    SensorData data;
    if (xQueueReceive(dataQueue, &data, portMAX_DELAY) != pdTRUE)
      continue;

    // 将数据发布到 MQTT 主题
    char payload[64];
    snprintf(payload, sizeof(payload), "{\"torque\":%.4f,\"speed\":%.4f,\"power\":%.4f}", data.torque, data.speed, data.power);
    mqttClient.publish("motor", payload);
  }
}

void setup()
{
  Serial.begin(9600);
  Serial1.begin(19200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  mqttClient.setServer(mqtt_server, 1883);
  while (!mqttClient.connected())
  {
    if (mqttClient.connect("esp32"))
    {
      Serial.println("Connected to MQTT server");
    }
    else
    {
      delay(1000);
      Serial.println("Connecting to MQTT server...");
    }
  }

  dataQueue = xQueueCreate(3, sizeof(SensorData));

  xTaskCreatePinnedToCore(mqttTask, "mqttTask", 4096, NULL, 1, &mqttTaskHandle, 1);
}

void loop()
{
  unsigned char SendComd[1] = {0X20};
  unsigned char ReceiveDate[15] = {};

  Serial1.write(SendComd, 1);
  delay(20);
  if (Serial1.available() > 0)
  {
    Serial1.readBytes(ReceiveDate, 15);
  }

  // 解码接收到的浮点数
  float torque = decodeFloat(&ReceiveDate[0]);
  float speed = decodeFloat(&ReceiveDate[5]);
  float power = decodeFloat(&ReceiveDate[10]);

  // 将数据存储到结构体中
  SensorData data = {torque, speed, power};

  // 将数据发送到队列
  xQueueSend(dataQueue, &data, portMAX_DELAY);

  delay(1000);
}
