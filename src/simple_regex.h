#include <string>
#include <exception>

#include <regex.h>
#include <string.h>

#ifndef __INCL_REGEX
#define __INCL_REGEX

#define REGEX_MATCH_LEN             64
#define ERROR_BUF_SIZE              128

class SimpleRegex
{
private:
    regex_t         regex;
    regmatch_t      pmatch[1];
    regoff_t        matchOffset;
    regoff_t        matchLength;

    char *          source;

public:
    SimpleRegex(std::string expression, char * pszSource);
    ~SimpleRegex();

    bool hasMoreMatches();

    std::string * nextMatch();
};

class regex_error : public std::exception
{
    private:
        std::string     message;

    public:
        regex_error() {
            this->message.assign("Regex error");
        }

        regex_error(const char * msg) {
            this->message.assign("Regex error: ");
            this->message.append(msg);
        }

        regex_error(const char * msg, const char * file, int line) {
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
