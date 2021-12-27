#include <string>
#include <exception>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifndef __INCL_REGEX_ERROR
#define __INCL_REGEX_ERROR

class xword_error : public std::exception
{
    private:
        std::string     message;

    public:
        xword_error() {
            this->message.assign("Regex error");
        }

        xword_error(const char * msg) {
            this->message.assign("Regex error: ");
            this->message.append(msg);
        }

        xword_error(const char * msg, const char * file, int line) {
            char lineNumBuf[8];

            sprintf(lineNumBuf, ":%d", line);

            this->message.assign("Regex error: ");
            this->message.append(msg);
            this->message.append(" at ");
            this->message.append(file);
            this->message.append(lineNumBuf);
        }

        virtual const char * what() const noexcept {
            return this->message.c_str();
        }

        static const char * buildMsg(const char * fmt, ...) {
            va_list     args;
            char *      buffer;

            va_start(args, fmt);
            
            buffer = (char *)malloc(strlen(fmt) + 80);
            
            vsprintf(buffer, fmt, args);
            
            va_end(args);

            return buffer;
        }
};

#endif
