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

