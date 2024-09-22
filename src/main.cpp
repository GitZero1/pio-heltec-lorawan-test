// main.ino

#include "Arduino.h"
#include "WiFi.h"
#include "images.h"
#include "RadioConfig.h"
#include <Wire.h>
#include "HT_SSD1306Wire.h"

// Display configuration
SSD1306Wire factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

bool resendflag = false;
bool deepsleepflag = false;
bool interrupt_flag = false;
String packet;
String show_lora = "lora data show";

void logo() {
    factory_display.clear();
    factory_display.drawXbm(0, 5, logo_width, logo_height, (const unsigned char *)logo_bits);
    factory_display.display();
}

void interrupt_GPIO0() {
    interrupt_flag = true;
}

void interrupt_handle(void) {
    if (interrupt_flag) {
        interrupt_flag = false;
        if (digitalRead(0) == 0) {
            deepsleepflag = true;
        }
    }
}

void VextON(void) {
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
}

void VextOFF(void) {
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, HIGH);
}

void setup() {
    Serial.begin(115200);
    VextON();
    delay(100);
    factory_display.init();
    factory_display.clear();
    factory_display.display();
    logo();
    delay(1000);
    factory_display.clear();

    attachInterrupt(0, interrupt_GPIO0, FALLING);
    lora_init();  // Initialize the LoRa radio

    packet = "waiting lora data!";
    factory_display.drawString(0, 10, packet);
    factory_display.display();
    delay(100);
    factory_display.clear();

    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
}

void loop() {
    interrupt_handle();

    if (deepsleepflag) {
        VextOFF();
        Radio.Sleep();
        SPI.end();
        pinMode(RADIO_DIO_1, ANALOG);
        pinMode(RADIO_NSS, ANALOG);
        pinMode(RADIO_RESET, ANALOG);
        pinMode(RADIO_BUSY, ANALOG);
        pinMode(LORA_CLK, ANALOG);
        pinMode(LORA_MISO, ANALOG);
        pinMode(LORA_MOSI, ANALOG);
        esp_sleep_enable_timer_wakeup(600 * 1000 * (uint64_t)1000);
        esp_deep_sleep_start();
    }

    if (resendflag) {
        state = STATE_TX;
        resendflag = false;
    }

    if (receiveflag && (state == LOWPOWER)) {
        receiveflag = false;
        packet = "R_data:";
        for (int i = 0; i < rxSize; i++) {
            packet += rxpacket[i];
        }
        factory_display.drawString(0, 0, show_lora);
        factory_display.drawString(0, 10, packet);
        factory_display.display();
        delay(10);
        factory_display.clear();
    }

    switch (state) {
        case STATE_TX:
            delay(1000);
            sprintf(txpacket, "hello world");
            Serial.printf("\r\nSending packet \"%s\", length %d\r\n", txpacket, strlen(txpacket));
            Radio.Send((uint8_t *)txpacket, strlen(txpacket));
            state = LOWPOWER;
            break;
        case STATE_RX:
            Serial.println("Entering RX mode");
            Radio.Rx(0);
            state = LOWPOWER;
            break;
        case LOWPOWER:
            Radio.IrqProcess();
            break;
        default:
            break;
    }
}
