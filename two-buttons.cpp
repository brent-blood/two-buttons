#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <SD.h>

/*
 * Chip-Select line for SD access.
 * I just noticed that I'm additionally using pin 33 for one of the
 * eponymous two buttons. As I only access the SD card during setup
 * this seems to work fine, but I should probably change that.
 */
#define CS 33

char* kindle_host = 0;          // system running the kindle app
uint16_t kindle_port = 9876;    // the port it's listening on

char* tivo_host = 0;            // tivo system
uint16_t tivo_port = 31339;     // tivo port

uint16_t debounce_delay = 500;  // ms after button push to wait
uint8_t dbg = 0;                // enable/disable debugging

/*
 * Debounce counters for the two buttons. Set to the next earliest time that
 * a poll of the button inputs should be valid.
 */
uint32_t prev_wait;
uint32_t next_wait;

/*
 * Used at various places to represent a cached value of millis()
 */
uint32_t now;

/*
 * Position for the potentiometer to determine what function should be used:
 * advance page in Kindle app, or 30s skip / skip commercials in Tivo
 */
uint16_t knob_pos;

/*
 * Pin assignments for the inputs. PREV AND NEXT are the push buttons while
 * FUNCTION is an input tied to a pot indicating what activity to control.
 */
const uint8_t NEXT = 33;
const uint8_t PREV = 27;
const uint8_t FUNCTION = A4;

/*
 * Handles for network protocol objects
 */
WiFiUDP udp;
WiFiClient tcp;

/*
 * Read a single word from a file, returning the number of characters read
 * before EOF, equals sign, or a carriage return was encountered.
 */
int get_word(File f, char* b, const size_t l) {
    char c;
    uint32_t i;
    for(i = 0; i < l - 1 && f.available(); i++) {
        c = f.read();
        // only stop if we hit an equals sign or a carriage return
        if(c == '=' || c == '\n') {
            b[i] = 0; // stash a null
            return i;
        }
        /*
         * Ignore spaces. This is naive as it doesn't check if the space is
         * followed by an equals or carriage return and so if a space appears
         * in the middle of a string it'll just silently skip it. So don't
         * put spaces in the middle of your strings.
         */
        if(c != ' ')
            b[i] = c; // stash character
        /*
         * Anyone that says string handling in C is easy is either truly a
         * genius or an idiot. Probably an idiot. I am also an idiot.
         */
    }
    return i;
}

void debug(const char* message) {
    if(!dbg) return;

    Serial.println(message);
}

void setup() {
    // create these on the stack so they don't linger forever
    const size_t l = 64;    // size of string buffers
    char key[l];            // keys pulled from config file
    char val[l];            // values pulled from config file

    char wifi_ap[l];        // name of wireless ESSID to connect to
    char wifi_pw[l];        // wireless secret

    char* tmp_str = 0;      // use for debug strings

    Serial.begin(9600);
    while (!Serial) { ; }

    delay(1000); // I've read this is a good idea.

    if (!SD.begin(CS)) {
        Serial.println("Card failed, or not present");
        while (1);
    }
    debug("SD card initialized.");

    // the leading / just means put it in the root of the SD card
    File f = SD.open("/two-buttons.cfg");
    if(!f) {
        Serial.println("Error opening SD config file: two-buttons.cfg");
        while(1);
    }
    debug("parsing config file");

    while(f.available()) {
        get_word(f, key, l); 
        get_word(f, val, l);

        /*
         * Use strncpy for variables that are only needed in this function.
         * Use strndup for variables that are needed outside. We never free them
         * because they're used until the mcu shuts down.
         */
        if(strncmp("WIFI_AP", key, l) == 0) {
            strncpy(wifi_ap, val, l);
        } else if(strncmp("WIFI_PW", key, l) == 0) {
            strncpy(wifi_pw, val, l);
        } else if(strncmp("KINDLE_HOST", key, l) == 0) {
            kindle_host = strndup(val, l);
        } else if(strncmp("KINDLE_PORT", key, l) == 0) {
            kindle_port = atoi(val);
        } else if(strncmp("TIVO_HOST", key, l) == 0) {
            tivo_host = strndup(val, l);
        } else if(strncmp("TIVO_PORT", key, l) == 0) {
            tivo_port = atoi(val);
        } else if(strncmp("DEBOUNCE_DELAY", key, l) == 0) {
            debounce_delay = (uint16_t)atoi(val);
        } else if(strncmp("DEBUG", key, l) == 0) {
            dbg = (uint8_t)atoi(val);
            tmp_str = (char*)malloc(l);
        }
    }

    // All done with the SD. Shut it down.
    f.close();

    // Just a little sanity check
    if(wifi_ap == NULL || wifi_pw == NULL) {
        Serial.println("No value provided for WIFI_AP or WIFI_PW");
        while(1);
    }

    if(kindle_host == NULL || tivo_host == NULL) {
        Serial.println("Must provide at least one of KINDLE_HOST or TIVO_HOST");
        while(1);
    }

    if(dbg) {
        sprintf(tmp_str, "kindle: %s:%d", kindle_host, kindle_port);
        debug(tmp_str);
        sprintf(tmp_str, "tivo: %s:%d", tivo_host, tivo_port);
        debug(tmp_str);
        sprintf(tmp_str, "debounce delay: %d", debounce_delay);
        debug(tmp_str);
        sprintf(tmp_str, "debug: %d", dbg);
        debug(tmp_str);
    }
    
    WiFi.begin(wifi_ap, wifi_pw);
    debug("Connecting to wireless network");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    if(dbg) {
        IPAddress ip = WiFi.localIP();
        sprintf(tmp_str, "Connected, IP address: %d.%d.%d.%d",
                ip[0], ip[1], ip[2], ip[3]);
        debug(tmp_str);
    }

    prev_wait = millis();
    next_wait = millis();

    // necessary for no good reason if you're only sending... but arduino!
    udp.begin(kindle_port);

    /*
     * Pull inputs high, connect them through ground, and test for LOW to
     * avoid trouble with floating inputs.
     */
    pinMode(NEXT, INPUT_PULLUP);
    pinMode(PREV, INPUT_PULLUP);

    pinMode(FUNCTION, INPUT);
}

void loop() {
    /*
     * I'm not sure if this matters much for Arduino and ESP32, but cache
     * a value of the main timer to use for each iteration through loop
     */
    now = millis();

    // check whether to poll to enable short circuiting digitalRead
    if(next_wait < now && digitalRead(NEXT) == LOW) {
        next_wait = now + debounce_delay;

        knob_pos = analogRead(FUNCTION);
        /*
         * Read the analog pin connected through the potentiometer and
         * divide the 4096 values up into ranges for the 3 states.
         */
        if(knob_pos > 2730) {
            debug("Sending 'next' to Kindle");
            udp.beginPacket(kindle_host, kindle_port);
            udp.write((uint8_t*)"next\n", 6);
            udp.endPacket();
        } else if(knob_pos > 1365) {
            debug("Sending 'IRCODE ACTION_D' to TiVo");
            tcp.connect(tivo_host, tivo_port);
            tcp.write("IRCODE ACTION_D\r\n");
            tcp.stop();
        } else {
            debug("Sending 'IRCODE ADVANCE' to TiVo");
            tcp.connect(tivo_host, tivo_port);
            tcp.write("IRCODE ADVANCE\r\n");
            tcp.stop();
        }
    }

    if(prev_wait < now && digitalRead(PREV) == LOW) {
        prev_wait = now + 500;

        /*
         * BACK for either Tivo function is the same thing so no need
         * to differentiate between them.
         */
        if(knob_pos > 2730) {
            debug("Sending 'prev' to Kindle");
            udp.beginPacket(kindle_host, kindle_port);
            /*
             * I didn't need to cast to uint8_t* for esp8266, but I do now
             * for esp32. Arduino!
             */
            udp.write((uint8_t*)"prev\n", 6);
            udp.endPacket();
        } else {
            debug("Sending 'IRCODE REPLAY' to TiVo");
            tcp.connect(tivo_host, tivo_port);
            tcp.write("IRCODE REPLAY\r\n");
            tcp.stop();
        }
    }
}
