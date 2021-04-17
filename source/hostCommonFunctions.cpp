#include <string>

#include "host.h"
#include "filehelpers.h"

#include "SimpleIni.h"

using namespace std;

CSimpleIniA settingsIni;

void Host::loadSettingsIni(){
    std::string settingsIniStr = get_file_contents(_logFilePrefix + "settings.ini");

	//File does not exist, fill string with defaults
	if(settingsIniStr.length() == 0 ){
        settingsIniStr = "[settings]\nstretch = 1\n";
	}

    settingsIni.LoadData(settingsIniStr);

    long stretchSetting = settingsIni.GetLongValue("settings", "stretch", (long)PixelPerfectStretch);
    if (stretchSetting <= (int)AltScreenStretch){
        stretch = (StretchOption) stretchSetting;
    }
}

void Host::saveSettingsIni(){
    //write out settings to persist
    settingsIni.SetLongValue("settings", "stretch", stretch);
    std::string settingsIniStr = "";
    settingsIni.Save(settingsIniStr, false);

    std::string iniPath = _logFilePrefix + "settings.ini";

    FILE * file = freopen(iniPath.c_str(), "w", stderr);
    if( file != NULL ) {
		//Initialize data
        fprintf(file, "%s", settingsIniStr.c_str());
		
        fflush(file);
        
        fclose(file);
	}
}

std::string Host::getCartDataFile(std::string cartDataKey) {
    return _logFilePrefix + "cdata/" + cartDataKey + ".p8d.txt";
}

std::string Host::getCartDataFileContents(std::string cartDataKey) {
    return get_file_contents(getCartDataFile(cartDataKey));
}

void Host::saveCartData(std::string cartDataKey, std::string contents) {
    FILE * file = freopen(getCartDataFile(cartDataKey).c_str(), "w", stderr);
    if( file != NULL ) {
		//Initialize data
        fprintf(file, "%s", contents.c_str());
		
        fflush(file);
        
        fclose(file);
	}
}