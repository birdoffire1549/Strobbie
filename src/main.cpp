/*
  Application Title: ... Strobbie
  Written by: .......... Scott Griffis
  Date: ................ 09-17-2024

  Description:
  This application was written for an ESP8266 module to enable it to control
  a strip of individually addressable LEDs for the purpose of performing 
  various light display routeens. The ESP8266 not only acts as the controller
  for the lights but it also serves up a WiFi Access Point that one can connect
  to and then access a webpage hosted by the device which also allows for 
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
#include <HtmlContent.h>
#include <Lighting.h>

// Constants defined
const String VERSION = "1.2.1";
const unsigned int PRIORITY_REDUCER =  70u;
const IPAddress AP_IP(192, 168, 1, 1);
const IPAddress SUBNET(255, 255, 255, 0);

// Define Services
DNSServer dnsServer;
ESP8266WebServer server(80);
Settings settings;

// General Function prototypes
void activateAPMode();
void handleRoot();

String deviceId = "";
int priorityCount = 0;

std::map<String, void (*)()> actions {
  {"allOff", &doAllOff},
  {"flashingColors", &doFlashingColors},
  // {"rotatingColorFade", &doRotatingColorFade},
  // {"oneDirectionChase", &doOneDirectionChase},
  // {"backAndForthChase", &doBackAndForthChase},
  // {"trainChase", &doTrainChase},
  // {"inwardChevronChase", &doInwardChevronChase},
  // {"outwardChevronChase", &doOutwardChevronChase},
  {"solidColors", &doSolidColors}
};

/**
 * -----
 * SETUP
 * ---------------------------------------------------------
 * This is the setup portion of the applicaiton.
 * Here is where the onetime initialization and setup of the
 * applicaiton and it components happens.
 */
void setup() { 
  // Generate Device ID Based On MAC Address
  deviceId = Utils::genDeviceIdFromMacAddr(WiFi.macAddress());

  // Load intial settings from flash memory
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
  initLighting();

  // Activate AP
  activateAPMode();
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

  // Activate captive portal
  dnsServer.start(53u, "*", AP_IP);

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
 * Application web control page.
 * 
 */
void handleRoot() {
  static CRGB tempColors[MAX_COLORS] = {actionColors[0]};
  static uint tempColorsSize = settings.getColorsSize();
  static ulong tempDelay = settings.getActionDelay();
  static String tempAction = settings.getActionName();

  if (server.method() == HTTP_GET) {
    for (uint i = 0; i < MAX_COLORS; i ++) {
      tempColors[i] = actionColors[i];
    }
  } else if (server.method() == HTTP_POST) {
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
          colorsString.concat(colorHex[i].substring(1));
        } else {
          colorsString.concat(":");
          colorsString.concat(colorHex[i].substring(1));
        }
      }

      // Save updated settings
      settings.setActionDelay((unsigned long) delay.toDouble());
      settings.setActionName(action);
      settings.setColors(colorsString);
      settings.setColorsSize(tempColorsSize);
      settings.saveSettings();

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
  
  // Set general page data
  String pageTemplate = HTML_MAIN_PAGE_TEMPLATE;
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
    String temp = HTML_COLOR_SELECTION_SECTION_TEMPLATE;
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