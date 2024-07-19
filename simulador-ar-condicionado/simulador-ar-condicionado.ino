/*
 * ReceiveDemo.cpp
 *
 * Demonstrates receiving IR codes with the IRremote library and the use of the Arduino tone() function with this library.
 * Long press of one IR button (receiving of multiple repeats for one command) is detected.
 * If debug button is pressed (pin connected to ground) a long output is generated, which may disturb detecting of repeats.
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2020-2024 Armin Joachimsmeyer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************************
 */

#include <Arduino.h>

#define IR_RECEIVE_PIN      2

#if !defined (FLASHEND)
#define FLASHEND 0xFFFF // Dummy value for platforms where FLASHEND is not defined
#endif

/*
 * Helper macro for getting a macro definition as string
 */
#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif

/*
 * Specify which protocol(s) should be used for decoding.
 * If no protocol is defined, all protocols (except Bang&Olufsen) are active.
 * This must be done before the #include <IRremote.hpp>
 */
// #define DECODE_DENON        // Includes Sharp
// #define DECODE_JVC
// #define DECODE_KASEIKYO
// #define DECODE_PANASONIC    // alias for DECODE_KASEIKYO
//#define DECODE_LG
// #define DECODE_ONKYO        // Decodes only Onkyo and not NEC or Apple
#define DECODE_NEC          // Includes Apple and Onkyo
// #define DECODE_SAMSUNG
// #define DECODE_SONY
// #define DECODE_RC5
// #define DECODE_RC6
// #define DECODE_BOSEWAVE
// #define DECODE_LEGO_PF
// #define DECODE_MAGIQUEST
// #define DECODE_WHYNTER
// #define DECODE_FAST
// #define DECODE_DISTANCE_WIDTH // Universal decoder for pulse distance width protocols
// #define DECODE_HASH         // special decoder for all protocols
// #define DECODE_BEO          // This protocol must always be enabled manually, i.e. it is NOT enabled if no protocol is defined. It prevents decoding of SONY!
#define RECORD_GAP_MICROS 16000  // always get the complete frame in the receive buffer, but this prevents decoding of SONY!
// etc. see IRremote.hpp
//

#define RAW_BUFFER_LENGTH 750
// #define NO_LED_FEEDBACK_CODE // saves 92 bytes program memory
// #define EXCLUDE_UNIVERSAL_PROTOCOLS // Saves up to 1000 bytes program memory.
// #define EXCLUDE_EXOTIC_PROTOCOLS // saves around 650 bytes program memory if all other protocols are active
// #define IR_REMOTE_DISABLE_RECEIVE_COMPLETE_CALLBACK // saves 32 bytes program memory

// MARK_EXCESS_MICROS is subtracted from all marks and added to all spaces before decoding,
// to compensate for the signal forming of different IR receiver modules. See also IRremote.hpp line 142.
// #define MARK_EXCESS_MICROS    20    // Adapt it to your IR receiver module. 40 is taken for the cheap VS1838 module her, since we have high intensity.

// #define RECORD_GAP_MICROS 12000 // Default is 8000. Activate it for some LG air conditioner protocols

// #define DEBUG // Activate this for lots of lovely debug output from the decoders.

#include <IRremote.hpp>

void handleOverflow();

/*
Esses defines sao para controlar o display de 7 segmentos, com base no seguinte
diagrama: https://softwareparticles.com/wp-content/uploads/2023/04/4-digit-7-segment-led-display-pinout.jpg
*/
#define A 9
#define B 4
#define C 10
#define D 11
#define E 12
#define Ff 6  // ja ha um macro F definido...
#define G 8   // ja ha um macro F definido...
#define D2 5
#define D3 3
#define D4 7

void limparDisplay() {
    digitalWrite(A, LOW);
    digitalWrite(B, LOW);
    digitalWrite(C, LOW);
    digitalWrite(D, LOW);
    digitalWrite(E, LOW);
    digitalWrite(Ff, LOW);
    digitalWrite(G, LOW);

    digitalWrite(D2, HIGH);
    digitalWrite(D3, HIGH);
    digitalWrite(D4, HIGH);
}
void mostrarDigito(int pos, int digito) {
    limparDisplay();

    digitalWrite(D4, HIGH);
    if (pos == 1) {
        digitalWrite(D2, LOW);
    } else {
        digitalWrite(D3, LOW);
    }

    switch (digito) {
        case 0:
            digitalWrite(A, HIGH);
            digitalWrite(B, HIGH);
            digitalWrite(C, HIGH);
            digitalWrite(D, HIGH);
            digitalWrite(E, HIGH);
            digitalWrite(Ff, HIGH);
            break;
        case 1:
            digitalWrite(B, HIGH);
            digitalWrite(C, HIGH);
            break;
        case 2:
            digitalWrite(A, HIGH);
            digitalWrite(B, HIGH);
            digitalWrite(G, HIGH);
            digitalWrite(E, HIGH);
            digitalWrite(D, HIGH);
            break;
        case 3:
            digitalWrite(A, HIGH);
            digitalWrite(B, HIGH);
            digitalWrite(G, HIGH);
            digitalWrite(C, HIGH);
            digitalWrite(D, HIGH);
            break;
        case 4:
            digitalWrite(Ff, HIGH);
            digitalWrite(B, HIGH);
            digitalWrite(G, HIGH);
            digitalWrite(C, HIGH);
            break;
        case 5:
            digitalWrite(A, HIGH);
            digitalWrite(Ff, HIGH);
            digitalWrite(G, HIGH);
            digitalWrite(C, HIGH);
            digitalWrite(D, HIGH);
            break;
        case 6:
            digitalWrite(A, HIGH);
            digitalWrite(C, HIGH);
            digitalWrite(D, HIGH);
            digitalWrite(E, HIGH);
            digitalWrite(Ff, HIGH);
            digitalWrite(G, HIGH);
            break;
        case 7:
            digitalWrite(A, HIGH);
            digitalWrite(B, HIGH);
            digitalWrite(C, HIGH);
            break;
        case 8:
            digitalWrite(A, HIGH);
            digitalWrite(B, HIGH);
            digitalWrite(C, HIGH);
            digitalWrite(D, HIGH);
            digitalWrite(E, HIGH);
            digitalWrite(Ff, HIGH);
            digitalWrite(G, HIGH);
            break;
        case 9:
            digitalWrite(A, HIGH);
            digitalWrite(B, HIGH);
            digitalWrite(C, HIGH);
            digitalWrite(Ff, HIGH);
            digitalWrite(G, HIGH);
            break;
    }
}

void mostrarC() {
    limparDisplay();
    digitalWrite(D4, LOW);
    digitalWrite(D3, HIGH);
    digitalWrite(D2, HIGH);

    digitalWrite(A, HIGH);
    digitalWrite(Ff, HIGH);
    digitalWrite(E, HIGH);
    digitalWrite(D, HIGH);
}

void setup() {
    Serial.begin(115200);
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    // In case the interrupt driver crashes on setup, give a clue
    // to the user what's going on.
    Serial.println(F("Enabling IRin..."));

    // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

    Serial.print(F("Ready to receive IR signals of protocols: "));
    printActiveIRProtocols(&Serial);
    Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));

    // infos for receive
    Serial.print(RECORD_GAP_MICROS);
    Serial.println(F(" us is the (minimum) gap, after which the start of a new IR packet is assumed"));
    Serial.print(MARK_EXCESS_MICROS);
    Serial.println(F(" us are subtracted from all marks and added to all spaces for decoding"));

    for (int i = 3; i < 13; i++) {
        pinMode(i, OUTPUT);
        digitalWrite(i, LOW);
    }
}

int temperatura = 22;
int iteracao = 0;

void loop() {
    iteracao = (iteracao + 1) % 3;  // mostrar os digitos um de cada vez (multiplexing)

    if (IrReceiver.decode()) {
        Serial.println();

        /*
         *
         */
        if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
            handleOverflow();
        } else {
            /*
             * No overflow here.
             * Stop receiver, print short info and send usage and start receiver again
             *****************************************************************************************************/

            /*
             * Print info
             */
            if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
                Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
                IrReceiver.printIRResultRawFormatted(&Serial, true);
            }
            if (IrReceiver.decodedIRData.protocol != UNKNOWN) {
                /*
                 * The info output for a successful receive
                 */
                IrReceiver.printIRResultShort(&Serial);
                IrReceiver.printIRSendUsage(&Serial);
            }
        }

        /*
         * !!!Important!!! Enable receiving of the next value, because receiving
         * has stopped after the end of the current received data packet.
         * Do it here, to preserve raw data for printing with printIRResultRawFormatted()
         */
        IrReceiver.resume();

        temperatura = (IrReceiver.decodedIRData.command) + 18;  // mapear os comandos para valores entre 18 e 30

        if (IrReceiver.decodedIRData.address == 0) {
            if (IrReceiver.decodedIRData.command == 0x10) {
                // do something
            } else if (IrReceiver.decodedIRData.command == 0x11) {
                // do something else
            }
        }
    }
    switch (iteracao) {
        case 0:
            mostrarDigito(1, temperatura / 10);
            break;
        case 1:
            mostrarDigito(2, temperatura % 10);
        case 2:
            mostrarC();
            break;
    }
}

void handleOverflow() {
    Serial.println(F("Overflow detected"));
    Serial.println(F("Try to increase the \"RAW_BUFFER_LENGTH\" value of " STR(RAW_BUFFER_LENGTH) " in " __FILE__));
    // see also https://github.com/Arduino-IRremote/Arduino-IRremote#compile-options--macros-for-this-library

    IrReceiver.stopTimer();
    delay(100);
    IrReceiver.restartTimer(100000);  // to compensate for 100 ms stop of receiver. This enables a correct gap measurement.
}