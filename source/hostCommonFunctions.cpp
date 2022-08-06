#include <string>

#include "host.h"
#include "hostVmShared.h"
#include "filehelpers.h"

#include "logger.h"

#include "SimpleIni.h"

#include "miniz.h"

#if LOAD_PACK_INS
#include "cartzip.h"
#endif

using namespace std;

CSimpleIniA settingsIni;

std::string defaultIni =
"[settings]\n"
"stretch = 1\n"
"resizekey = 1\n"
"kbmode = 0\n"
"menustyle = 0\n"
"bgcolor = 0\n";

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

void Host::unpackCarts(){

	#if LOAD_PACK_INS
	if(packinloaded == Unloaded){
		Logger_Write("unzipping pack in carts to p8carts\n");
		
		//based on https://github.com/richgel999/miniz/issues/38
		
		mz_zip_archive zip_archive;
		memset(&zip_archive, 0, sizeof(zip_archive));

		// init zip file
		mz_zip_reader_init_mem(&zip_archive, source_carts_zip, source_carts_zip_len, 0);
		
		int fileCount = (int)mz_zip_reader_get_num_files(&zip_archive);
		if (fileCount == 0)
		{
			mz_zip_reader_end(&zip_archive);
		}
		mz_zip_archive_file_stat file_stat;
		if (!mz_zip_reader_file_stat(&zip_archive, 0, &file_stat)) 
		{
			mz_zip_reader_end(&zip_archive);
		}
		// Get root folder
		string base = _cartDirectory; // path delim on end

		// Get and print information about each file in the archive.
		for (int i = 0; i < fileCount; i++)
		{
			if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) continue;
			if (mz_zip_reader_is_file_a_directory(&zip_archive, i)) continue; // skip directories for now
			string fileName = base + file_stat.m_filename; // make path relative
			Logger_Write(fileName.c_str());
			Logger_Write("\n");
			string destFile = fileName; // make full dest path

			mz_zip_reader_extract_to_file(&zip_archive, i, destFile.c_str(), 0);
		}

		// Close the archive, freeing any resources it was using
		mz_zip_reader_end(&zip_archive);
		
		packinloaded = Loaded;
		
	}else{
		Logger_Write("packins already loaded\n");
	}
	
	#endif
	
	
}

void Host::loadSettingsIni(){
    std::string settingsIniStr = get_file_contents(_logFilePrefix + "settings.ini");
	
	//File does not exist, fill string with defaults
	if(settingsIniStr.length() == 0 ){
		#if LOAD_PACK_INS
        settingsIniStr = defaultIni + "packinloaded = 0\n";
		#else
		settingsIniStr = defaultIni;
		#endif
	}

    settingsIni.LoadData(settingsIniStr);
	
	//stretch
    long stretchSetting = settingsIni.GetLongValue("settings", "stretch", (long)PixelPerfectStretch);
    if (stretchSetting <= (int)AltScreenStretch){
        stretch = (StretchOption) stretchSetting;
    }
	
	#if LOAD_PACK_INS
	long packinloadedSetting = settingsIni.GetLongValue("settings", "packinloaded", (long)Unloaded);
	packinloaded = (PackinLoadOption) packinloadedSetting;
	#endif
	
	//resize hotkey
	long resizekeySetting = settingsIni.GetLongValue("settings", "resizekey", (long)YesResize);
	resizekey = (ResizekeyOption) resizekeySetting;
	
	//kbmode
    long kbmodeSetting = settingsIni.GetLongValue("settings", "kbmode", (long)Emoji);
	kbmode = (KeyboardOption) kbmodeSetting;
	
	//bgcolor
	long menustyleSetting = settingsIni.GetLongValue("settings", "menustyle", (long)Classic);
	menustyle = (MenuStyleOption) menustyleSetting;
	
	//bgcolor
	long bgcolorSetting = settingsIni.GetLongValue("settings", "bgcolor", (long)Gray);
	bgcolor = (BgColorOption) bgcolorSetting;
}

void Host::saveSettingsIni(){
    //write out settings to persist
	
    settingsIni.SetLongValue("settings", "stretch", stretch);
	
	#if LOAD_PACK_INS
    settingsIni.SetLongValue("settings", "packinloaded", packinloaded);
	#endif
	
    settingsIni.SetLongValue("settings", "resizekey", resizekey);
	
    settingsIni.SetLongValue("settings", "kbmode", kbmode);
    settingsIni.SetLongValue("settings", "menustyle", menustyle);
    settingsIni.SetLongValue("settings", "bgcolor", bgcolor);
	
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

size_t Host::getFileContents(std::string fileName, char* buffer) {
	std::string absPath = _logFilePrefix + "cdata/" + fileName;
    //get_file_bin_contents(absPath, buffer, length);
	FILE * file = freopen(absPath.c_str(), "rb", stderr);
    if( file != NULL ) {
		//Initialize data
		size_t len;
		fread(&len, sizeof(len), 1, file);
		fread(buffer, sizeof(char), len, file);
        
        fclose(file);

		return len;
	}

	return 0;
}

void Host::writeBufferToFile(std::string fileName, char* buffer, size_t length) {
	std::string absPath = _logFilePrefix + "cdata/" + fileName;
    FILE * file = freopen(absPath.c_str(), "w", stderr);
    if( file != NULL ) {
		//Initialize data
		fwrite(&length, sizeof(length), 1, file);
		fwrite(buffer, sizeof(char), length, file);
        
        fclose(file);
	}
}

//settings

int Host::getSetting(std::string sname) {
    
	if(sname == "kbmode"){ //why cant you use strings in switch statements in c++ :(
		Logger_Write("Returning KB mode setting\n");
		return kbmode;
	}else if(sname == "resizekey"){
		Logger_Write("Returning resize key setting\n");
		return resizekey;
	}else if(sname == "stretch"){
		Logger_Write("Returning Stretch setting\n");
		return stretch;
	}else if(sname == "menustyle"){
		Logger_Write("Returning menu style setting\n");
		return menustyle;
	}else if(sname == "bgcolor"){
		Logger_Write("Returning bg color setting\n");
		return bgcolor;
	}else if(sname == "p8_bgcolor"){
		
		Logger_Write("Returning pico 8 bg color setting\n");
		std::string bgcolorstr = std::to_string(bgcolor) + "\n";
		Logger_Write(bgcolorstr.c_str());
		switch(bgcolor)
		{
			case Gray: return   05;
			case Black: return  00;
			case Blue: return   01;
			case Green: return  03;
			case Purple: return 13;
			case White: return  07;
			default:     return 02;
		}
	}else if(sname == "p8_textcolor"){
		Logger_Write("Returning pico 8 text color setting\n");
		if(bgcolor == White){
			return 0;
		}else{
			return 7;
		}
	}else{
		Logger_Write("Setting ");
		Logger_Write(sname.c_str());
		Logger_Write(" not found, returning 0!");
		return 0;
	}
	
}

void Host::setSetting(std::string sname, int sval) {
	if(sname == "kbmode"){ //why cant you use strings in switch statements in c++ :(
		Logger_Write("setting KB mode\n");
		kbmode = (KeyboardOption) sval;
	}else if(sname == "resizekey"){
		Logger_Write("setting resize hotkey\n");
		resizekey = (ResizekeyOption) sval;
	}else if(sname == "stretch"){
		Logger_Write("setting Stretch to");
		std::string stringval = std::to_string(sval);
		Logger_Write(stringval.c_str());
		Logger_Write("\n");
		
		
		stretch = (StretchOption) sval;
		
		//force change stretch
		forceStretch(stretch);
	}else if(sname == "menustyle"){
		Logger_Write("setting menustyle\n");
		menustyle = (MenuStyleOption) sval;
		
	}else if(sname == "bgcolor"){
		Logger_Write("setting bgcolor\n");
		bgcolor = (BgColorOption) sval;
		
	}else if(sname == "packinloaded"){
		Logger_Write("setting packinloaded\n");
		
		#if LOAD_PACK_INS
		packinloaded = (PackinLoadOption) sval;
		#endif
		
	}else{
		Logger_Write("Setting ");
		Logger_Write(sname.c_str());
		Logger_Write(" not found!");
	}
	
}