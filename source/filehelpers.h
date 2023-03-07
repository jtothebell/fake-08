#include <string>
#include <vector>

std::string get_file_contents(std::string filename);

std::vector<char> get_file_as_buffer(std::string filename);

std::string get_first_four_chars(std::string filename);

std::string getDirectory(const std::string& fname);

bool hasEnding (std::string const &fullString, std::string const &ending);

bool isCartFile (std::string const &fullString);

bool isCPostFile (std::string const &fullString);

bool isHiddenFile (std::string const &fullString);

bool isAbsolutePath (std::string const &path);

std::string getFileExtension(std::string const &path);
