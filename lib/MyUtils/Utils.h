#ifndef Utils_h
    #define Utils_h

    #include <WString.h>
    #include <MD5Builder.h>
    #include <map>

    std::map<String, uint8> hexToDecDigitMap {
        {"0", 0u},
        {"1", 1u},
        {"2", 2u},
        {"3", 3u},
        {"4", 4u},
        {"5", 5u},
        {"6", 6u},
        {"7", 7u}, 
        {"8", 8u},
        {"9", 9u},
        {"a", 10u},
        {"b", 11u},
        {"c", 12u},
        {"d", 13u},
        {"e", 14u},
        {"f", 15u}
    };

    std::map<uint8, String> decToHexDigitMap {
        {0u, "0"},
        {1u, "1"},
        {2u, "2"},
        {3u, "3"},
        {4u, "4"},
        {5u, "5"},
        {6u, "6"},
        {7u, "7"}, 
        {8u, "8"},
        {9u, "9"},
        {10u, "a"},
        {11u, "b"},
        {12u, "c"},
        {13u, "d"},
        {14u, "e"},
        {15u, "f"}
    };

    class Utils {
        private:

        public:
            static String hashString(String string);
            static String genDeviceIdFromMacAddr(String macAddress);
            static String rgbDecimalsToHex(uint8 red, uint8 green, uint8 blue);
            static String decimalTo8BitHex(uint8 dec);
            static uint8 hexTo8BitDecimal(String hex);
            static void split(String string, char separator, String *storage, int sizeOfStorage);
    };

#endif
