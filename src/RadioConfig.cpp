// RadioConfig.cpp

#include "RadioConfig.h"

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];
int16_t Rssi, rxSize;
bool receiveflag = false;
States_t state;  // Define the state variable here

static RadioEvents_t RadioEvents;

void OnTxDone(void) {
    Serial.print("TX done......");
    state = STATE_RX;
}

void OnTxTimeout(void) {
    Radio.Sleep();
    Serial.print("TX Timeout......");
    state = STATE_TX;
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
    Rssi = rssi;
    rxSize = size;
    memcpy(rxpacket, payload, size);
    rxpacket[size] = '\0';
    Radio.Sleep();
    Serial.printf("\r\nReceived packet \"%s\" with Rssi %d, length %d\r\n", rxpacket, Rssi, rxSize);
    Serial.println("Wait to send next packet");
    receiveflag = true;
}

void lora_init(void) {
    Mcu.begin();

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxDone = OnRxDone;

    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

    state = STATE_TX;
}
