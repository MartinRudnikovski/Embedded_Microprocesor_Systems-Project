E#include "esp_camera.h"
#include <WiFi.h>
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"
#include <BlynkSimpleEsp32.h>
#include <Servo.h> 



Servo servo;

//Motor driver interface pins
unsigned const int IN1pin = 14;
unsigned const int IN2pin = 15;
unsigned const int IN3pin = 13;
unsigned const int IN4pin = 12;


//Servo motor interface pin
unsigned const int servoPin = 2;


//Blynk authentication credential
char auth[] = "YAtzJaGfBks7chjHUJ-Hy-q5NFom_4kQ";
//Wifi connection
const char* ssid = "xxx";
const char* password = "xxx";

void startCameraServer();


//================================Motor driver commands==============================
void forward()
{
  digitalWrite(IN1pin, HIGH);
  digitalWrite(IN2pin, LOW);
  digitalWrite(IN3pin, HIGH);
  digitalWrite(IN4pin, LOW);
}
void backward()
{
  digitalWrite(IN1pin, LOW);
  digitalWrite(IN2pin, HIGH);
  digitalWrite(IN3pin, LOW);
  digitalWrite(IN4pin, HIGH);
}
void turnLeft()
{
  digitalWrite(IN1pin, LOW);
  digitalWrite(IN2pin, HIGH);
  digitalWrite(IN3pin, HIGH);
  digitalWrite(IN4pin, LOW);
}
void turnRight()
{
  digitalWrite(IN1pin, HIGH);
  digitalWrite(IN2pin, LOW);
  digitalWrite(IN3pin, LOW);
  digitalWrite(IN4pin, HIGH);
}
void standStill()
{
  digitalWrite(IN1pin, LOW);
  digitalWrite(IN2pin, LOW);
  digitalWrite(IN3pin, LOW);
  digitalWrite(IN4pin, LOW);
}
void setMotorDriver()
{
  pinMode(IN1pin, OUTPUT);
  pinMode(IN2pin, OUTPUT);
  pinMode(IN3pin, OUTPUT);
  pinMode(IN4pin, OUTPUT);
}
//================================Motor driver commands==============================//


//================================Virtual pins listeners==============================
BLYNK_WRITE(V1)
{
    int x = param.asInt();
    if (param.asInt() == 1)
      backward();
    else
      standStill();
}
BLYNK_WRITE(V2)
{
    int x = param.asInt();
    if (param.asInt() == 1)
      forward();
    else
      standStill();
}
BLYNK_WRITE(V3)
{
    int x = param.asInt();
    if (param.asInt() == 1)
      turnLeft();
    else
      standStill();
}
BLYNK_WRITE(V4)
{
    int x = param.asInt();
    if (param.asInt() == 1)
      turnRight();
    else
      standStill();
}
BLYNK_WRITE(V5)
{
    int x = param.asInt();
    servo.write(180-x);
}
BLYNK_WRITE(V6)
{
    int x = param.asInt();
    if (param.asInt() == 1){
      servo.write(90);
    }
}

//================================Virtual pins listeners==============================//


void setup() {
  
//=================================Start camera stream through router===========================
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

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
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
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

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
//=================================Start camera stream through router===========================//

  //Blynk setup
  Blynk.begin(auth, ssid, password, IPAddress(192,168,0,13), 8080);


  //Motors setup
  setMotorDriver();

  
  //Servo motor setup
  ledcSetup(8, 5000, 8);
  servo.attach(servoPin, 8, 0, 180);
  servo.write(90);
}

void loop() {
  Blynk.run();
}
