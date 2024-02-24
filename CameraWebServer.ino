#include "esp_camera.h"
#include <WiFi.h>
#include <PubSubClient.h>

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER

#define TOPIC_PREFIX "b6610502145"
#define TOPIC_TEST TOPIC_PREFIX "/test"

#include "camera_pins.h"

const char* WIFI_SSID = "KUWIN-IOT";
const char* WIFI_PASS = "";

// mqtt broker ip address
const char* MQTT_BROKER = "iot.cpe.ku.ac.th";
const char* MQTT_USER = "b6610502145";
const char* MQTT_PASS = "parinya.ao@ku.th";
// end mqtt broker ip address
WiFiClient wifiClient;
PubSubClient mqtt(MQTT_BROKER, 1883, wifiClient);
uint32_t last_publish = 0;
//

// about capture interval
#define RESOLUTION_X 640
#define RESOLUTION_Y 480
const int capture_interval = 1000;
//

// function connect wifi
void connect_wifi() {
  printf("WiFi MAC address is %s\n", WiFi.macAddress().c_str());
  printf("Connecting to WiFi %s.\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    fflush(stdout);
    delay(500);
  }
  Serial.println("\nWiFi success.");
}
// end function connect wifi

// function connect mqtt
void connect_mqtt() {
  Serial.print("Connecting to MQTT broker at ");
  Serial.println(MQTT_BROKER);
  if (!mqtt.connect("", MQTT_USER, MQTT_PASS)) {
    Serial.println("Failed to connect to MQTT broker. Please check your credentials and try again.");
    for (;;) {} // wait here forever
  }
  mqtt.setCallback(mqtt_callback);
  mqtt.subscribe(TOPIC_TEST);
  Serial.println("MQTT broker connected.");
}
// end functon connecct mqtt

// function mqtt call back
void mqtt_callback(char* topic, byte* payload, unsigned int length) {

}
// end function mqtt call back

void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  connect_wifi();

  connect_mqtt();
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");


  // publish
  last_publish = 0;
}

void loop() {
  // 
  static unsigned long last_capture = 0;
  unsigned long now = millis();

  if (now - last_capture >= capture_interval) {
    Serial.println("Taking a photo...");
    last_capture = now;

    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    esp_camera_fb_return(fb);

    Serial.println("Photo taken!");
  }

  // test publish
  mqtt.loop();
  uint32_t now = millis();
  if (now - last_publish >= 3000) {
    String text_test = "ping hello world";
    String payload = text_test;
    Serial.println("publishing text");
    mqtt.publish(TOPIC_TEST, payload.c_str());
    last_publish = now;
    Serial.println("published successfully");
  }
}
 