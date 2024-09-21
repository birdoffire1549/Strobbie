/*
  Application Title: ... Strobbie
  Written by: .......... Scott Griffis
  Date: ................ 09-17-2024

  Description:
  This application was written for an ESP8266 module to enable it to control
  a strip of individually addressable LEDs for the purpose of performing 
  various light display routeens. The ESP8266 not only acts as the controller
  for the lights but it also serves up a WiFi Access Point that one can connect
  to and then access a webpage hosted the the device which also allows for 
  the lights to be controlled in various ways.

  Originally this software was written so that a light strip places inside a white
  pool noodle could be used as a colored strobe light for a halloween display.
*/

#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h> 
#include <map>

#include <Utils.h>
#include <IpUtils.h>
#include <Settings.h>

#define DATA_PIN 5
#define NUM_LEDS 11

const String VERSION = "1.1.0";
const unsigned int MAX_COLORS = 3u;
const unsigned int PRIORITY_REDUCER =  70u;
const IPAddress AP_IP(192, 168, 1, 1);
const IPAddress SUBNET(255, 255, 255, 0);

// Define the array of leds
CRGB leds[NUM_LEDS];
DNSServer dnsServer;
ESP8266WebServer server(80);

String deviceId = "";

CRGB rgbStringToColor(String rgbString, uint beginIndex);
void activateAPMode();
void handleRoot();
void handleNotFound();

// Action functions defined
void doFlashingColors();
void doRotatingColorFade();
void doSolidColors();

// State variables
ulong actionDelay = 70ul;
void (*currentAction)() = &doFlashingColors;
CRGB actionColors[MAX_COLORS];
uint actionColorsSize = 1u;

int priorityCount = 0;

std::map<String, void (*)()> actions{
  {"flashingColors", &doFlashingColors},
  {"rotatingColorFade", &doRotatingColorFade},
  {"solidColors", &doSolidColors}
};

Settings settings;

/**
 * -----
 * SETUP
 * ---------------------------------------------------------
 * This is the setup portion of the applicaiton.
 * Here is where the onetime initialization and setup of the
 * applicaiton and it components happens.
 */
void setup() { 
  settings.loadSettings();
  actionDelay = settings.getActionDelay();
  currentAction = actions[settings.getActionName()];
  actionColorsSize = settings.getColorsSize();
  String colorsString = settings.getColors();
  String colorsArray[MAX_COLORS] = {"000000"};
  Utils::split(colorsString, ':', colorsArray, MAX_COLORS);
  for (uint i = 0; i < MAX_COLORS; i++) {
    actionColors[i] = rgbStringToColor(colorsArray[i], 0u);
  }

  // Initialize LEDs
  actionColors[0] = CRGB::Red;
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.clearData();

  // Generate Device ID Based On MAC Address
  deviceId = Utils::genDeviceIdFromMacAddr(WiFi.macAddress());

  // Activate AP
  activateAPMode();

  // Activate web server
  server.on("/", handleRoot); 
  server.onNotFound(handleRoot);
  server.begin();
}

/**
 * ----
 * LOOP
 * -------------------------------------------------
 * The main looping body of the application.
 * This is the repetitive portion of the application
 * which drives it to perform all functionality
 * for the duration of the device's runtime 
 * post setup.
 */
void loop() {
  static ulong counter = 0;
  if ((counter++) % PRIORITY_REDUCER == 0) { 
    dnsServer.processNextRequest();
    server.handleClient();
  }
  
  // Priority process
  currentAction();
}

/**
 * Puts the device into AP Mode so that user can
 * connect via WiFi directly to the device to configure
 * the device.
 */
void activateAPMode() {
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setOutputPower(20.5F);
  WiFi.setHostname(deviceId.c_str());
  WiFi.mode(WiFiMode::WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_IP, SUBNET);

  String ssid = "Strobbie_";
  ssid.concat(deviceId);
  String pwd = "Str0bb13";
  
  WiFi.softAP(ssid, pwd);
  dnsServer.start(53u, "*", AP_IP);
}

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
}

/**
 * Application web control page.
 * 
 */
void handleRoot() {
  static CRGB tempColors[MAX_COLORS] = {actionColors[0]};
  static uint tempColorsSize = actionColorsSize;
  static ulong tempDelay = actionDelay;
  static String tempAction = "flashingColors";
  
  if (server.method() == HTTP_POST) {
    String formDo = server.arg("do");

    // Handle various Form related 'DO' Actions...
    if (String("add").equalsIgnoreCase(formDo)) { // <---------------------------- ADD Button
      String colorHex[tempColorsSize];
      for (uint i = 0; i < tempColorsSize; i++) {
        String argName = "selectColor";
        argName.concat(String(i));
        colorHex[i] = server.arg(argName);
      }
      for (uint i = 0; i < tempColorsSize; i++) {
        if (!colorHex[i].isEmpty()) {
          tempColors[i] = rgbStringToColor(colorHex[i], 1u);
        }
      }
      if (tempColorsSize < MAX_COLORS) {
        tempColorsSize ++;
        tempColors[tempColorsSize - 1] = CRGB::Black;
      }
      tempDelay = (ulong)server.arg("changeDelay").toDouble();
      tempAction = server.arg("action");
    } else if (formDo.startsWith("remove")) { // <-------------------------- REMOVE Button
      String colorNum = formDo.substring(7, 8);
      String colorHex[tempColorsSize];
      for (uint i = 0; i < tempColorsSize; i++) {
        String argName = "selectColor";
        argName.concat(String(i));
        colorHex[i] = server.arg(argName);
      }
      for (uint i = 0; i < tempColorsSize; i++) {
        if (!colorHex[i].isEmpty()) {
          tempColors[i] = rgbStringToColor(colorHex[i], 1);
        }
      }

      uint iColorNum = colorNum.toInt();
      if (iColorNum > 0u && iColorNum < tempColorsSize) {
        for (uint i = iColorNum; i < tempColorsSize; i++) {
          tempColors[i] = tempColors[i + 1];
        }
        tempColorsSize --;
      }
      tempDelay = (ulong)server.arg("changeDelay").toDouble();
      tempAction = server.arg("action");
    } else if (String("update").equalsIgnoreCase(formDo)) { // <--------------------- UPDATE Button
      // Handle 'Update' Button click
      String action = server.arg("action");
      String delay = server.arg("changeDelay");
      String colorHex[tempColorsSize];
      String colorsString = "";

      // Get color settings
      for (uint i = 0; i < tempColorsSize; i++) {
        String argName = "selectColor";
        argName.concat(String(i));
        colorHex[i] = server.arg(argName);
        if (colorsString.isEmpty()) {
          colorsString.concat(colorHex[i]);
        } else {
          colorsString.concat(":");
          colorsString.concat(colorHex[i]);
        }
      }

      // Save updated settings
      settings.setActionDelay((unsigned long) delay.toDouble());
      settings.setActionName(action);
      settings.setColors(colorsString);
      settings.setColorsSize(tempColorsSize);

      // TODO: Verify incoming data!!!
      
      // Set all application states
      tempAction = action;
      currentAction = actions[action];
      actionDelay = (unsigned long) delay.toDouble();
      tempDelay = actionDelay;
      for (uint i = 0; i < tempColorsSize; i++) {
        if (!colorHex[i].isEmpty()) {
          CRGB color = rgbStringToColor(colorHex[i], 1); 

          actionColors[i] = color;
          tempColors[i] = color;
        }
      }
      actionColorsSize = tempColorsSize;
    }
  }
  
  // Define the page template
  String pageTemplate = 
    "<!DOCTYPE html><html><body>"
      "<h1>Strobbie V${appVersion}</h1>"
      "<form action=\"/\" method=\"post\">"
        "<label for=\"action\">Action:</label>"
        "<select id=\"action\" name=\"action\">"
          "<option value=\"solidColors\" ${solidColors_sel}>Solid Color Light</option>"
          "<option value=\"flashingColors\" ${flashingColors_sel}>Flashing Color</option>"
          "<option value=\"rotatingColorFade\" ${rotatingColorFade_sel}>Rotating Color Fader</option>"
        "</select>"
        "<br />"
        "<label for=\"changeDelay\">Change delay in millis:</label>"
        "<input type=\"number\" id=\"changeDelay\" name=\"changeDelay\" value=${changeDelay}><br />"
        "Add another color to action: "
        "<button type=\"submit\" name=\"do\" value=\"add\" ${add_disable}>Add</button> ${add_disableMessage}"
        "<hr />"
        "${selectedColors}"
        "<br /><br /><button type=\"submit\" name=\"do\" value=\"update\">Update</button>"
      "</form>"
    "</body></html>"
  ;

  // Define a single color selection item template
  String selectedColorTemplate = 
    "<p>"
      "<label for=\"selectColor${colorNumber}\">Color #${colorNumber}:</label>"
      "<input type=\"color\" id=\"selectColor${colorNumber}\" name=\"selectColor${colorNumber}\" value=\"#${selectColor}\">"
      "<button type=\"submit\" name=\"do\" value=\"remove:${colorNumber}\" ${remove_disable}>Remove</button>"
    "</p><hr />"
  ;
  
  // Set general page data
  pageTemplate.replace("${appVersion}", VERSION);

  // Set the appropriate Action that is Selected
  pageTemplate.replace("${flashingColors_sel}", (tempAction == "flashingColors") ? "selected" : "");
  pageTemplate.replace("${rotatingColorFade_sel}", (tempAction == "rotatingColorFade") ? "selected" : "");
  pageTemplate.replace("${solidColors_sel}", (tempAction == "solidColors") ? "selected" : "");

  // Set the action delay time
  pageTemplate.replace("${changeDelay}", String(tempDelay));
  
  // Diable add button when max colors reached
  pageTemplate.replace("${add_disable}", tempColorsSize == MAX_COLORS ? "disabled" : "");
  pageTemplate.replace("${add_disableMessage}", tempColorsSize == MAX_COLORS ? "* Max colors reached." : "");

  // Build out the Color Section of the page
  String colorSection = "";
  for (uint i = 0u; i < tempColorsSize; i++) {
    String temp = selectedColorTemplate;
    temp.replace("${colorNumber}", String(i));
    temp.replace("${colorNumber}", String(i));
    temp.replace("${colorNumber}", String(i));
    temp.replace("${colorNumber}", String(i));
    temp.replace("${colorNumber}", String(i));

    // Diable removal of first color
    temp.replace("${remove_disable}", i == 0 ? "disabled" : "");
    
    // Set the selected color
    temp.replace("${selectColor}", Utils::rgbDecimalsToHex((uint8)tempColors[i].red, tempColors[i].green, tempColors[i].blue));
    colorSection.concat(temp);
  }
  pageTemplate.replace("${selectedColors}", colorSection);

  // Send the built page
  server.send(200, "text/html", pageTemplate);
}

/**
 * This function is used to flash the LEDs, this can be a 
 * single color on and off or between any number of colors.
 * 
 */
void doFlashingColors() {
  static CRGB lastColor = CRGB::Black;
  static ulong lastChange = 0ul;

  if (millis() - lastChange >= actionDelay) {
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
    // Serial.printf("Color0: rgb(%d, %d, %d)\n", nextColor.red, nextColor.green, nextColor.blue);
    // Display the change
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = nextColor;
    }
    FastLED.show();

    // Update state
    lastColor = nextColor;
    lastChange = millis();
  }
}

/**
 * Fades from one color to the next where the colors overlap some.
 * Uses about half of the LEDs when color solid.
 * 
 */
void doRotatingColorFade() {
  // FIXME: This is a soon to come feature.
}

/**
 * 
 */
void doSolidColors() {
  uint largeGroupSize = NUM_LEDS / actionColorsSize;
  uint smallGroupSize = largeGroupSize % actionColorsSize == 0u ? largeGroupSize / actionColorsSize : (largeGroupSize / actionColorsSize) + 1u;

  uint colorIndex = 0u;
  // Iterate the LEDs
  for (uint i = 1u; i <= NUM_LEDS; i++) {
    // If a color group is filled and there are more colors, use next color otherwise go back to first color
    colorIndex = (i % smallGroupSize == 0u ? (colorIndex < actionColorsSize - 1u ? colorIndex + 1u : colorIndex = 0u) : colorIndex);
    leds[i - 1u] = actionColors[colorIndex];
  }
  FastLED.show();
}