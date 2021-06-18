#include <string>
#include <regex>

struct charset
{
    // Convert between UTF-8 strings and 8-bit PICO-8 strings
    static std::string utf8_to_pico8(std::string const &str);
    static std::string pico8_to_utf8(std::string const &str);

    // Map 8-bit PICO-8 characters to UTF-32 codepoints
    static std::u32string_view to_utf32[256];

    // Map 8-bit PICO-8 characters to UTF-8 string views
    static std::string_view to_utf8[256];

private:
    static std::regex static_init();
    static std::regex utf8_regex;
};