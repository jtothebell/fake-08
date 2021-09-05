#include <string>

std::string get_file_contents(std::string filename);

std::string getDirectory(const std::string& fname);

bool hasEnding (std::string const &fullString, std::string const &ending);

bool isCartFile (std::string const &fullString);

bool isHiddenFile (std::string const &fullString);

bool isAbsolutePath (std::string const &path);

std::string getFileExtension(std::string const &path);
