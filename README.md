# Strobbie

## Description
This application was written for an ESP8266 module to enable it to control
a strip of individually addressable LEDs for the purpose of performing 
various light display routeens. The ESP8266 not only acts as the controller
for the lights but it also serves up a WiFi Access Point that one can connect
to and then access a webpage hosted by the device which also allows for 
the lights to be controlled in various ways.

Strobbie stores its settings in Flash Memory so if it is unplugged it will boot
up with the settings it had prior.

Originally this software was written so that a light strip places inside a white
pool noodle could be used as a colored strobe light for a halloween display.