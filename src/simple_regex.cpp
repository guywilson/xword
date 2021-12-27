#include <iostream>
#include <string>
#include <stdlib.h>

#include "simple_regex.h"

using namespace std;

SimpleRegex::SimpleRegex(string expression, char * pszSource)
{
    int rtn = regcomp(&regex, expression.c_str(), REG_EXTENDED | REG_NEWLINE);

    if (rtn) {
        char szErrorString[ERROR_BUF_SIZE];

        regerror(rtn, &regex, szErrorString, ERROR_BUF_SIZE);

        throw regex_error(
            regex_error::buildMsg(
                "Failed to compile regular expression '%s': %s", 
                expression.c_str(), 
                szErrorString), 
            __FILE__, 
            __LINE__);
    }

    source = pszSource;
}

SimpleRegex::~SimpleRegex()
{
    regfree(&regex);
}

bool SimpleRegex::hasMoreMatches()
{
    bool                rtn;

    if (!regexec(&regex, source, 1, pmatch, 0)) {
        matchOffset = pmatch[0].rm_so;
        matchLength = pmatch[0].rm_eo - pmatch[0].rm_so;

        if (matchLength > REGEX_MATCH_LEN) {
            throw regex_error("Match too long for buffer");
        }

        rtn = true;
    }
    else {
        rtn = false;
    }

    return rtn;
}

string * SimpleRegex::nextMatch()
{
    string * match = new string(&source[matchOffset], matchLength);

    source += pmatch[0].rm_eo;

    return match;
}
