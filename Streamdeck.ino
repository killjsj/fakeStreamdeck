#include "ShiftIn.h"
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <Arduino.h>
// #include <controller.h>
#include <FastLED.h>  //include

// lib:Fastled,ArduinoJson Board:esp32s2
#define buttonpin 6  // pins
#define a1 14        // pins
#define a2 15        // pins
#define a3 16        // pins
#define sound 13     // pins
#define dac 18
#define x 11  // pins
#define y 12  // pins
#define EEPROM_SIZE 1024
#define NUM_LEDS 16  // define
ShiftIn<2> shift;
int addr = 0;      // EEPROM addr
uint16_t sum = 0;  // checksum
StaticJsonDocument<500> jsonBuffer;
typedef struct quetran {
  int buttonpress[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  // torch 1 - 16,i6 dev by user
  int adc[6] = { 0, 0, 0, 0, 0, 0 };                                         //  a1,a2,a3,sound,a1|x,a2|y

} quetran;  // queuetranfrom
typedef struct jsonio {
  /* data */
  int rgb[48] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  // r,g,b,r,g,b
} jsonio;
QueueHandle_t queue;
QueueHandle_t uartin;
CRGB leds[NUM_LEDS];  // var
void setup() {
  // put your setup code here, to run once:
  Serial.begin(128000);
  Serial1.begin(115200);  // garbage throw at here...
  Serial.printf("Starting system init,LEDS init and shift init \n");
  shift.begin(34, 36, 33, 35);
  FastLED.addLeds<WS2812, 7, GRB>(leds, NUM_LEDS);  // leds init
  FastLED.setBrightness(128);
  fill_gradient_RGB(leds, 0, CRGB(68, 32, 121), 15, CRGB(44, 121, 32));  // leds rgb show
  FastLED.show();
  delay(1000);
  for (int i = 0; !(i == 16); i++) {
    leds[i] = CRGB::Black;
    FastLED.show();
    delay(100);
  }  // clear leds
  Serial.printf("RGB Inited! Initing Freertos....\n");
  uartin = xQueueCreate(1, sizeof(jsonio));
  queue = xQueueCreate(1, sizeof(quetran));
  xTaskCreate(main_uart_hand,
              "uart",
              2048,
              NULL,
              5,
              NULL);
  xTaskCreate(main_input_hand,
              "input",
              2048,
              NULL,
              5,
              NULL);
  xTaskCreate(main_rgb_hand,
              "rgb",
              2048,
              NULL,
              5,
              NULL);
  Serial.printf("FreeRTOS inited! System init complete!\n");
  Serial.printf("ST\n");  // start transfrom
}

jsonio json(String json) {
  StaticJsonDocument<500> jsonBuffer;
  DeserializationError error = deserializeJson(jsonBuffer, const_cast<char *>(json.c_str()));
  if (error) {
    Serial.print(F("DeserializeJson failed: "));
    Serial.println(error.f_str());
  }
  jsonio thisoutput;
  for (int i = 0; i <= 47; i++) {
    thisoutput.rgb[i] = jsonBuffer["rgb"][i];
  }
  return thisoutput;
}

void main_input_hand(void *p) {
  Serial.println("adcoutput");
  while (1) {
    quetran thisout;
    unsigned int sft = shift.read();
    for (int i = 15; i >= 0; i--) {
      thisout.buttonpress[i] = bitRead(sft,i);// 15->1 14->2 13->3 12->4 11->5 10->6 9->7 8->6 7->16 6->15 5->14 4->13 3->9 2->10 1->11 0->12
    }
    thisout.adc[0] = adc_read_val(a1);  // a1
    thisout.adc[1] = adc_read_val(a2);
    thisout.adc[2] = adc_read_val(a3);
    thisout.adc[3] = adc_read_val(sound);
    thisout.adc[4] = adc_read_val(x);
    thisout.adc[5] = adc_read_val(y);
    xQueueOverwrite(queue, (void *)&thisout);
    vTaskDelay(5);
  }
}
void main_rgb_hand(void *p) {
  while (1) {
    jsonio bein;
    if (xQueueReceive(uartin, (void *)&bein, 0) == errQUEUE_EMPTY) {
    } else {
      int nowled = 0;
      for (int i = 0; i <= 47; i = i + 3) {
        leds[nowled] = CRGB(bein.rgb[i], bein.rgb[i + 1], bein.rgb[i + 2]);
        FastLED.show();
        nowled++;
      }
    }
  }
}
uint32_t adc_read_val(int chan) {

  int samplingFrequency = 260;  // 采样频率（可调）
  long sum = 0;                 // 采样和
  float samples = 0.0;          // 采样平均值

  for (int i = 0; i < samplingFrequency; i++) {
    sum += analogRead(chan);
    delayMicroseconds(40);
  }
  samples = sum / (float)samplingFrequency;
  return samples;  //单位(mV)
}
void main_uart_hand(void *p) {
  while (1) {
    String in;
    if (Serial.available() > 0) {
      in = Serial.readStringUntil('\n');
      jsonio inj = json(in);
      xQueueOverwrite(uartin, &inj);
    }
    quetran beout;

    if (xQueueReceive(queue, (void *)&beout, 30) == errQUEUE_EMPTY) {
    } else {

      JsonArray button = jsonBuffer.createNestedArray("button");
      for (int i = 0; i <= 15; i++) {
        button.add(beout.buttonpress[i]);
      }
      JsonArray adc = jsonBuffer.createNestedArray("adc");
      for (int i = 0; i <= 5; i++) {
        adc.add(beout.adc[i]);
      }
      String uartoutput;
      serializeJson(jsonBuffer, uartoutput);
      Serial.println(uartoutput+"Eof");
    }
    delayMicroseconds(5);
  }
}


void loop() {}  // Use freertos....
