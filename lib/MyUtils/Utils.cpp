#include "Utils.h"

/**
 * Function used to perform a MD5 Hash on a given string
 * the result is the MD5 Hash.
 * 
 * @param string The string to hash as String.
 * 
 * @return Returns the generated MD5 Hash as String.
*/
String Utils::hashString(String string) {
    MD5Builder builder = MD5Builder();
    builder.begin();
    builder.add(string);
    builder.calculate();
    
    return builder.toString();
}

/**
 * Generates a six character Device ID based on the
 * given macAddress.
 * 
 * @param macAddress The device's MAC Address as String.
 * 
 * @return Returns a six digit Device ID as String.
*/
String Utils::genDeviceIdFromMacAddr(String macAddress) {
    String result = hashString(macAddress);
    int len = result.length();
    if (len > 6) {
        result = result.substring((len - 6), len);
    }
    result.toUpperCase();

    return result;
}

String Utils::decimalTo8BitHex(uint8 dec) {
    String result = "";
    uint8 d1 = dec >> 4;
    uint8 d2 = dec & 0x0F;

    result.concat(decToHexDigitMap[d1]);
    result.concat(decToHexDigitMap[d2]);

    return result;
}

uint8 Utils::hexTo8BitDecimal(String hex) {
    hex.toLowerCase();
    uint8 result = 0u;
    result = hexToDecDigitMap[hex.substring(0, 1)] << 4;
    result = result | hexToDecDigitMap[hex.substring(1, 2)];

    return result;
}

String Utils::rgbDecimalsToHex(uint8 red, uint8 green, uint8 blue) {
    String result = "";
    result.concat(decimalTo8BitHex(red));
    result.concat(decimalTo8BitHex(green));
    result.concat(decimalTo8BitHex(blue));

    return result;
}