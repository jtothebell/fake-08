#include <string>
#include <fstream>
#include <vector>

//http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
std::string get_file_contents(std::string filename){
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return(contents);
    }

  return "";
}

//https://stackoverflow.com/questions/5420317/reading-and-writing-binary-file
std::vector<unsigned char> get_file_buffer(std::string filename){
  std::ifstream in(filename, std::ios::binary);

  std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(in), {});

  return buffer;
}

//https://stackoverflow.com/a/8518855
std::string getDirectory(const std::string& fname){
     size_t pos = fname.find_last_of("\\/");
     return (std::string::npos == pos)
         ? ""
         : fname.substr(0, pos);
}

bool isAbsolutePath (std::string const &path) {
    if (path.length() == 0) {
        return false;
    }

    if (path[0] == '/') {
        return true;
    } 
    
    size_t colonPos = path.find_first_of(":");
    size_t slashPos = path.find_first_of("/");

    if (colonPos != std::string::npos && slashPos == (colonPos + 1)){
        return true;
    }
    
    return false;
}

std::string getFileExtension(std::string const &path) {
    size_t pos = path.find_last_of(".");
     return (std::string::npos == pos)
         ? ""
         : path.substr(pos);
}

bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

bool isHiddenFile (std::string const &fullString) {
    if (fullString.length() >= 0) {
        return (0 == fullString.compare (0, 2, "._"));
    } else {
        return false;
    }
}

bool isCartFile (std::string const &fullString) {
    return !isHiddenFile(fullString) && 
        (hasEnding(fullString, ".p8") || hasEnding(fullString, ".png"));
}

