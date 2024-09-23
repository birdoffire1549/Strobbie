#ifndef LightingEffects_h
    #define LightingEffects_h
    
    #define NUM_LEDS 11
    #define DATA_PIN 5

    #include <FastLED.h>
    #include <Utils.h>

    const unsigned int MAX_COLORS = 3u;

    void doAllOff();
    void doFlashingColors();
    void doRotatingColorFade();
    void doSolidColors();
    void doOneDirectionChase();
    void doBackAndForthChase();
    void doTrainChase();
    void doInwardChevronChase();
    void doOutwardChevronChase();
    CRGB rgbStringToColor(String rgbString, uint beginIndex);

    std::map<String, void (*)()> actions {
        {"allOff", &doAllOff},
        {"flashingColors", &doFlashingColors},
        {"oneDirectionChase", &doOneDirectionChase},
        // {"rotatingColorFade", &doRotatingColorFade},
        {"backAndForthChase", &doBackAndForthChase},
        // {"trainChase", &doTrainChase},
        {"inwardChevronChase", &doInwardChevronChase},
        // {"outwardChevronChase", &doOutwardChevronChase},
        {"solidColors", &doSolidColors}
    };

    // Define the array of leds
    CRGB leds[NUM_LEDS];

    // State variables
    ulong actionDelay = 70ul;
    void (*currentAction)() = &doFlashingColors;
    CRGB actionColors[MAX_COLORS];
    uint actionColorsSize = 1u;    

    void initLighting() {
        // Initialize LEDs
        FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
        FastLED.clear();
        FastLED.clearData();
    };

    /**
     * This function is used to flash the LEDs, this can be a 
     * single color on and off or between any number of colors.
     * 
     */
    void doFlashingColors() {
        static CRGB lastColor = CRGB::Black;
        static ulong lastChange = 0ul;
        static ulong milliWather = 0ul;

        if (millis() - lastChange >= actionDelay || milliWather > millis()) {
            milliWather = millis();
            CRGB nextColor;
            // Time to do an update
            if (actionColorsSize == 1) {
                if (lastColor == actionColors[0]) {
                    nextColor = CRGB::Black;
                } else {
                    nextColor = actionColors[0];
                }
            } else {
                // Find the next color from actionColors
                bool nextFound = false;
                for (uint i = 0u; i < actionColorsSize; i++) {
                    if (actionColors[i] == lastColor) {
                        nextFound = true;
                        if (i == actionColorsSize - 1) {
                            nextColor = actionColors[0];
                        } else {
                            nextColor = actionColors[(i + 1)];
                        }

                        break;
                    }
                }
                if (!nextFound) {
                    nextColor = actionColors[0];
                }
            }

            // Display the change
            for (uint i = 0; i < NUM_LEDS; i++) {
                leds[i] = nextColor;
            }
            FastLED.show();

            // Update state
            lastColor = nextColor;
            lastChange = millis();
        }
    };

    /**
     * Fades from one color to the next where the colors overlap some.
     * Uses about half of the LEDs when color solid.
     * 
     */
    void doRotatingColorFade() {
        // TODO: Coming Soon...
    };

    void doOneDirectionChase() {
        static int headIndex = 0;
        static uint currColorIndex = 0u;
        static ulong lastChange = 0ul;
        static ulong milliWatcher = 0ul;

        if (millis() - lastChange >= actionDelay || milliWatcher > millis()) {
            milliWatcher = millis();
            for (uint i = 0u; i < NUM_LEDS; i++) {
                if (headIndex < NUM_LEDS) {
                    // Move the group forward...
                    leds[i] = (i != headIndex ? CRGB::Black : actionColors[currColorIndex]);
                } else {
                    // Pick next color and reset the head...
                    currColorIndex = (currColorIndex + 1 == actionColorsSize ? 0u : currColorIndex + 1u);
                    headIndex = 0; 
                }
            }
            headIndex ++;
            lastChange = millis();
            FastLED.show();
        }
    }

    void doBackAndForthChase() {
        static int headIndex = -1;
        static bool forward = true;
        static uint currColorIndex = 0u;
        static ulong lastChange = 0ul;
        static ulong milliWatcher = 0ul;

        if (millis() - lastChange >= actionDelay || milliWatcher > millis()) {
            milliWatcher = millis();
            for (uint i = 0u; i <= NUM_LEDS; i++) {
                if ((headIndex < NUM_LEDS && forward) || (headIndex > -1 && !forward)) {
                    // Move the group forward...
                    leds[i] = (i != headIndex ? CRGB::Black : actionColors[currColorIndex]);
                } else if (forward) {
                    forward = false; 
                } else {
                    // Pick next color and reset the head...
                    currColorIndex = (currColorIndex + 1 == actionColorsSize ? 0u : currColorIndex + 1u); 
                    forward = true;
                }
            }
            if (forward) {
                headIndex ++;
            } else {
                headIndex --;
            }
            lastChange = millis();
            FastLED.show();
        }
    }

    void doTrainChase() {

        // TODO: Coming Soon...
    }

    void doInwardChevronChase() {
        static int firstIndex = 0;
        static uint secondIndex = NUM_LEDS - 1;
        static uint actionColorIndex = 0u;
        static ulong lastChange = 0ul;
        static ulong rolloverWatcher = 0ul;

        if (millis() - lastChange >= actionDelay || rolloverWatcher > millis()) {
            rolloverWatcher = millis();

            uint firstLastIndex = ((NUM_LEDS - 1) / 2u) + ((NUM_LEDS - 1) % 2u);
            uint halfLength = firstLastIndex + 1u;

            for (uint i = 0u; i <= halfLength; i++) {
                if (firstIndex != firstLastIndex + 1) {
                    leds[i] = (i == firstIndex ? actionColors[actionColorIndex] : CRGB::Black);
                    uint n = NUM_LEDS - 1 - i;
                    leds[n] = (n == secondIndex ? actionColors[actionColorIndex] : CRGB::Black);    
                } else {
                    firstIndex = -1;
                    secondIndex = NUM_LEDS;
                    actionColorIndex = (actionColorIndex < actionColorsSize - 1 ? actionColorIndex + 1 : 0u);
                }
            }
            firstIndex ++;
            secondIndex --;
            FastLED.show();
            lastChange = millis();
        }
    }

    void doOutwardChevronChase() {
        // TODO: Coming Soon...
    }

    /**
     * Turns off all the LEDS by settings their values to
     * Black.
     * 
     */
    void doAllOff() {
        for (uint i = 0u; i < MAX_COLORS; i ++) {
            leds[i] = CRGB::Black;
        }
        FastLED.show();
    };

    /**
     * Displays the colors chosen in a sequencial unchanging
     * solid pattern all at once.
     */
    void doSolidColors() {
        uint largeGroupSize = NUM_LEDS / actionColorsSize;
        uint smallGroupSize = largeGroupSize % actionColorsSize == 0u ? largeGroupSize / actionColorsSize : (largeGroupSize / actionColorsSize) + 1u;

        uint colorIndex = 0u;
        // Iterate the LEDs
        for (uint i = 1u; i <= NUM_LEDS; i++) {
            // If a color group is filled and there are more colors, use next color otherwise go back to first color
            colorIndex = (i % smallGroupSize == 0u ? (colorIndex < actionColorsSize - 1u ? colorIndex + 1u : 0u) : colorIndex);
            leds[i - 1u] = actionColors[colorIndex];
        }
        FastLED.show();
    };

    /**
     * UTILITY FUNCTION
     * ----------------
     * This is a utility function which converts a RGB String to a CRGB color object.
     * This method allows for a begin index to be supplied which determines where 
     * parsing of the color hex digits begins in the given string. This can be useful
     * particularly when working with color codes that begin with a '#'.
     * 
     * @param rgbString - The RGB hex code as a String.
     * @param beginIndex - The index from which to begin parsing the RGB hex code as uint.
     */
    CRGB rgbStringToColor(String rgbString, uint beginIndex) {
        uint8 red = Utils::hexTo8BitDecimal(rgbString.substring(beginIndex, beginIndex + 2));
        uint8 green = Utils::hexTo8BitDecimal(rgbString.substring(beginIndex + 2, beginIndex + 4));
        uint8 blue = Utils::hexTo8BitDecimal(rgbString.substring(beginIndex + 4, beginIndex + 6));
                
        CRGB color; 
        color.setRGB(red, green, blue);

        return color;
    };
#endif