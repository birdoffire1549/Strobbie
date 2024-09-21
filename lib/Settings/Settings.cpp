/*
  Settings - A class to contain, maintain, store and retreive settings needed
  by the application. This Class object is intented to be the sole manager of 
  data used throughout the applicaiton. It handles storing both volitile and 
  non-volatile data, where by definition the non-volitile data is persisted
  in flash memory and lives beyond the running life of the software and the 
  volatile data is lost and defaulted each time the software runs.

  Written by: ... Scott Griffis
  Date: ......... 10-01-2023
*/

#include <Settings.h>

Settings::Settings() {
    defaultSettings();
}

/**
 * Performs a factory default on the information maintained by this class
 * where that the data is first set to its factory default settings then
 * it is persisted to flash.
 * 
 * @return Returns true if successful saving defaulted settings otherwise
 * returns false as bool.
*/
bool Settings::factoryDefault() {
    defaultSettings();
    bool ok = saveSettings();

    return ok;
}

/**
 * Used to save or persist the current value of the non-volatile settings
 * into flash memory.
 *
 * @return Returns a true if save was successful otherwise a false as bool.
*/
bool Settings::saveSettings() {
    strcpy(nvSettings.sentinel, hashNvSettings(nvSettings).c_str()); // Ensure accurate Sentinel Value.
    EEPROM.begin(sizeof(NVSettings));

    EEPROM.wipe(); // usage seemd to grow without this.
    EEPROM.put(0, nvSettings);
    
    bool ok = EEPROM.commit();

    EEPROM.end();
    
    return ok;
}

/**
 * Used to load the settings from flash memory.
 * After the settings are loaded from flash memory the sentinel value is 
 * checked to ensure the integrity of the loaded data. If the sentinel 
 * value is wrong then the contents of the memory are deemed invalid and
 * the memory is wiped and then a factory default is instead performed.
 * 
 * @return Returns true if data was loaded from memory and the sentinel 
 * value was valid.
*/
bool Settings::loadSettings() {
    bool ok = false;
    // Setup EEPROM for loading and saving...
    EEPROM.begin(sizeof(NVSettings));

    // Persist default settings or load settings...
    delay(15);

    /* Load from EEPROM if applicable... */
    if (EEPROM.percentUsed() >= 0) { // Something is stored from prior...
        EEPROM.get(0, nvSettings);
        if (strcmp(nvSettings.sentinel, hashNvSettings(nvSettings).c_str()) != 0) { // Memory is corrupt...
            EEPROM.wipe();
            factoryDefault();
        } else { // Memory seems ok...
            ok = true;
        }
    }
    
    EEPROM.end();

    return ok;
}

/*
=================================================================
Private Functions BELOW
=================================================================
*/

/**
 * #### PRIVATE ####
 * This function is used to set or reset all settings to 
 * factory default values but does not persist the value 
 * changes to flash.
*/
void Settings::defaultSettings() {
    // Default the settings..
    strcpy(nvSettings.actionName, factorySettings.actionName);
    nvSettings.actionDelay = factorySettings.actionDelay;
    strcpy(nvSettings.colors, factorySettings.colors);
    nvSettings.colorsSize = factorySettings.colorsSize;
}

/**
 * #### PRIVATE ####
 * Used to provide a hash of the given NonVolatileSettings.
 * 
 * @param nvSet An instance of NonVolatileSettings to calculate a hash for.
 * 
 * @return Returns the calculated hash value as String.
*/
String Settings::hashNvSettings(NVSettings nvSet) {
    String content = "";
    content = content + String(nvSet.actionName);
    content = content + String(nvSet.actionDelay);
    content = content + String(nvSet.colors);
    content = content + String(nvSet.colorsSize);
    
    MD5Builder builder = MD5Builder();
    builder.begin();
    builder.add(content);
    builder.calculate();

    return builder.toString();
}

String Settings::getActionName() { return nvSettings.actionName; }
unsigned long Settings::getActionDelay() { return nvSettings.actionDelay; }
String Settings::getColors() { return nvSettings.colors; }
unsigned int Settings::getColorsSize() { return nvSettings.colorsSize; }

void Settings::setActionName(String actionName) { strcpy(nvSettings.actionName, actionName.c_str()); }
void Settings::setActionDelay(unsigned long actionDelay) { nvSettings.actionDelay = actionDelay; }
void Settings::setColors(String colors) { strcpy(nvSettings.colors, colors.c_str()); }
void Settings::setColorsSize(unsigned int colorsSize) { nvSettings.colorsSize = colorsSize; }