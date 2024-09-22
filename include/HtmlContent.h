#ifndef HtmlContent_h
    #define HtmlContent_h

    #include <WString.h>
    #include <pgmspace.h>

    const char PROGMEM HTML_MAIN_PAGE_TEMPLATE[] = {
        "<!DOCTYPE html>"
        "<html><body>"
        "<h1>Strobbie V${appVersion}</h1>"
        "<form action=\"/\" method=\"post\">"
            "<label for=\"action\">Action:</label>"
            "<select id=\"action\" name=\"action\">"
            "<option value=\"allOff\" ${allOff_sel}>Lights Off</option>"
            "<option value=\"solidColors\" ${solidColors_sel}>Solid Color Light</option>"
            "<option value=\"flashingColors\" ${flashingColors_sel}>Flashing Color</option>"
            "<option value=\"oneDirectionChase\" ${oneDirectionChase_sel}>One Direction Chase</option>"
            "<option value=\"backAndForthChase\" ${backAndForthChase_sel}>Back & Forth Chase</option>"
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
    };

    const char PROGMEM HTML_COLOR_SELECTION_SECTION_TEMPLATE[] = {
        "<p>"
        "<label for=\"selectColor${colorNumber}\">Color #${colorNumber}:</label>"
        "<input type=\"color\" id=\"selectColor${colorNumber}\" name=\"selectColor${colorNumber}\" value=\"#${selectColor}\">"
        "<button type=\"submit\" name=\"do\" value=\"remove:${colorNumber}\" ${remove_disable}>Remove</button>"
        "</p>"
        "<hr />"
    };
#endif