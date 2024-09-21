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

#ifndef Settings_h
    #define Settings_h

    #include <WString.h>
    #include <ESP_EEPROM.h>
    #include <MD5Builder.h>

    class Settings {
        private:
            struct NVSettings {
                char             actionName     [100]    ;
                unsigned long    actionDelay             ;
                char             colors         [100]    ;
                unsigned int     colorsSize              ;
                char             sentinel       [33]     ; // Holds a 32 MD5 hash + 1
            } nvSettings;

            struct NVSettings factorySettings = {
                "flashingColors", // <------------ actionName
                70ul, // <------------------------ actionDelay
                "0000FF:000000:000000", // <------ colors
                1u, // <-------------------------- colorsSize
                "NA" // <------------------------- sentinel
            };

            void defaultSettings();
            String hashNvSettings(struct NVSettings nvSet);

        public:
            Settings();

            bool loadSettings();
            bool saveSettings();
            bool factoryDefault();

            // Getters defined below
            String           getActionName     ();
            unsigned long    getActionDelay    ();
            String           getColors         ();
            unsigned int     getColorsSize     ();

            // Setters defined below
            void     setActionName     (String actionName);
            void     setActionDelay    (unsigned long delayMillis);
            void     setColors         (String colorsString);
            void     setColorsSize     (unsigned int size);
    };
#endif