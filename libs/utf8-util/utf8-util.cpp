#include <stdexcept>
#include "utf8-util.h"

namespace utf8
{
    namespace impl
    {
        int processLeading (char c, char32_t& initialcp, bool throwOnErr)
        {
            if ((c & 0x80) == 0)
            {
                initialcp = c;
                return  0;
            }

            if ((c & 0xe0) == 0xc0)
            {
                initialcp = c & 0x1f;
                return 1;
            }
            else if ((c & 0xf0) == 0xe0)
            {
                initialcp = c & 0x0f;
                return 2;
            }
            else if ((c & 0xf8) == 0xf0)
            {
                initialcp = c & 0x07;
                return 3;
            }
            else
            {
                if (throwOnErr)
                    throw std::runtime_error("invalid continuation count in leading byte");
                return -1;
            }
        }

        int countContinuations (char c, bool throwOnErr)
        {
            if ((c & 0x80) == 0)
                return  0;
            else if ((c & 0xe0) == 0xc0)
                return 1;
            else if ((c & 0xf0) == 0xe0)
                return 2;
            else if ((c & 0xf8) == 0xf0)
                return 3;
            else
            {
                if (throwOnErr)
                    throw std::runtime_error("invalid continuation count in leading byte");

                return -1;
            }
        }

        bool isContinuation (char c)
        {
            return (c & 0xc0) == 0x80;
        }

        bool addContinuation (char c, char32_t& cp, bool throwOnError)
        {
            if ((c & 0xc0) == 0x80)
            {
                cp = (cp << 6) + (c & 0x3f);
                return true;
            }
            if (throwOnErr)
                throw std::runtime_error("expected continuation byte not found");
            return false;
        }
    }
}

