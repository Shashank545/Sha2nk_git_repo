// MIT License
// https://github.com/gonzalocasas/arduino-uno-dragino-lorawan/blob/master/LICENSE
// Based on examples from https://github.com/matthijskooijman/arduino-lmic
// Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman

#include <lmic.h>
#include <hal/hal.h>
#include <DHT.h>
#include <DHT_U.h>

DHT dht(A0, DHT11);
/*************************************
 * TODO: Change the following keys
 * NwkSKey: network session key, AppSKey: application session key, and DevAddr: end-device address
 *************************************/
static const u1_t NWKSKEY[16] = {0x9F, 0x72, 0x81, 0xD8, 0xE1, 0x8D, 0xF0, 0xF1, 0xA8, 0xF6, 0x06, 0x10, 0x98, 0x9D, 0xB3, 0xEF};
static const u1_t APPSKEY[16] = {0x58, 0xB6, 0x96, 0x0C, 0xEE, 0x89, 0xF8, 0xAA, 0xA9, 0x47, 0x42, 0x60, 0x47, 0x48, 0xEC, 0x6A};
static const u4_t DEVADDR = 0x26011B2A;


// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 10;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9,
    .dio = {2, 6, 7},
};

void onEvent (ev_t ev) {
    if (ev == EV_TXCOMPLETE) {
        Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
        // Schedule next transmission
        os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
    }
}

void do_send(osjob_t* j){
    // Payload to send (uplink)

   /* int value = analogRead(A0);
     Serial.println("value");
    Serial.println(value);
    
    message[0] = highByte(value);
    Serial.println("message[0]");
    Serial.println(message[0]);
    
    message[1] = lowByte(value);
    Serial.println("message[1]");
    Serial.println(message[1]);*/


    uint32_t humidity = dht.readHumidity(false)*100;
    uint32_t temperature = dht.readTemperature(false)*100;

    //Serial.println("Humidity " String(humidity));
   // Serial.println("Temperature " String(temperature));

    byte payload[4];
    payload[0] = highByte(humidity);
    payload[1] = lowByte(humidity); 
    payload[2] = highByte(temperature); 
    payload[3] = lowByte(temperature);

    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, payload, sizeof(payload), 0);
        Serial.println(F("Sending uplink packet..."));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("Starting..."));

    // LMIC init
    os_init();

    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Set static session parameters.
    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);

    // Start job
    do_send(&sendjob);
}

void loop() {
    os_runloop_once();
    
    }
