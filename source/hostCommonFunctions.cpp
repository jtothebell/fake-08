#include <string>

#include "host.h"
#include "hostVmShared.h"
#include "filehelpers.h"

#include "SimpleIni.h"

using namespace std;

CSimpleIniA settingsIni;

void Host::setUpPaletteColors(){
    _paletteColors[0] = COLOR_00;
	_paletteColors[1] = COLOR_01;
	_paletteColors[2] = COLOR_02;
	_paletteColors[3] = COLOR_03;
	_paletteColors[4] = COLOR_04;
	_paletteColors[5] = COLOR_05;
	_paletteColors[6] = COLOR_06;
	_paletteColors[7] = COLOR_07;
	_paletteColors[8] = COLOR_08;
	_paletteColors[9] = COLOR_09;
	_paletteColors[10] = COLOR_10;
	_paletteColors[11] = COLOR_11;
	_paletteColors[12] = COLOR_12;
	_paletteColors[13] = COLOR_13;
	_paletteColors[14] = COLOR_14;
	_paletteColors[15] = COLOR_15;

	for (int i = 16; i < 128; i++) {
		_paletteColors[i] = {0, 0, 0, 0};
	}

	_paletteColors[128] = COLOR_128;
	_paletteColors[129] = COLOR_129;
	_paletteColors[130] = COLOR_130;
	_paletteColors[131] = COLOR_131;
	_paletteColors[132] = COLOR_132;
	_paletteColors[133] = COLOR_133;
	_paletteColors[134] = COLOR_134;
	_paletteColors[135] = COLOR_135;
	_paletteColors[136] = COLOR_136;
	_paletteColors[137] = COLOR_137;
	_paletteColors[138] = COLOR_138;
	_paletteColors[139] = COLOR_139;
	_paletteColors[140] = COLOR_140;
	_paletteColors[141] = COLOR_141;
	_paletteColors[142] = COLOR_142;
	_paletteColors[143] = COLOR_143;
}

Color* Host::GetPaletteColors(){
    return _paletteColors;
}

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