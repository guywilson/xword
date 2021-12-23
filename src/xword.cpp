#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <regex.h>

using namespace std;

#define MODE_XWORD                  1
#define MODE_ANAGRAM                2

#define DICTIONARY_FILE             "./dictionary.db"

#define ERROR_BUF_SIZE              128
#define REGEX_MATCH_LEN             64


void inline printUsage() {
    cout << "Usage:" << endl;
    cout << "    xcode <options> [input string]" << endl << endl;
    cout << "    -x crossword mode, list all possible matches for input string" << endl;
    cout << "    -a anagram mode, list all anagrams for the word in input string" << endl << endl;
}

int main(int argc, char *argv[])
{
    int             mode = 0;
    int             rtn;
    int             inputLen;
    char *          pszInput;
    char *          pszDictionary;
    FILE *          fptr;
    char            szErrorString[ERROR_BUF_SIZE];
    char            szMatch[REGEX_MATCH_LEN];
    uint32_t        dictionaryLen;
    regex_t         regex;
    regmatch_t      pmatch[1];
    regoff_t        matchOffset;
    regoff_t        matchLength;

    if (argc > 2) {
        for (int i = 1;i < argc;i++) {
            if (argv[i][0] == '-') {
                if (strncmp(&argv[i][1], "x", 2) == 0) {
                    mode = MODE_XWORD;
                }
                else if (strncmp(&argv[i][1], "a", 2) == 0) {
                    mode = MODE_ANAGRAM;
                }
                else {
                    cout << "Invalid argument: " << &argv[i][0] << endl << endl;
                    printUsage();
                    exit(-1);
                }
            }
        }
    }
    else {
        printUsage();
        exit(-1);
    }

    pszInput = strdup(&argv[2][0]);

    inputLen = (int)strlen(pszInput);

    /*
    ** Make sure the input string is lower case...
    */
    for (int i = 0;i < inputLen;i++) {
        pszInput[i] = tolower(pszInput[i]);
    }

    fptr = fopen(DICTIONARY_FILE, "rt");

    if (fptr == NULL) {
        cout << "Failed to open dictionary file '" << DICTIONARY_FILE << "'" << endl;
        exit(-1);
    }

    fseek(fptr, 0L, SEEK_END);
    dictionaryLen = ftell(fptr);
    fseek(fptr, 0L, SEEK_SET);

    pszDictionary = (char *)malloc(dictionaryLen);

    if (pszDictionary == NULL) {
        cout << "Failed to allocate memory for dictionary" << endl;
        fclose(fptr);
        exit(-1);
    }

    fread(pszDictionary, 1, dictionaryLen, fptr);

    fclose(fptr);

    if (mode == MODE_XWORD) {
        string regexInitialiser = "^("; 

        for (int j = 0;j < inputLen;j++) {
            if (pszInput[j] == '?') {
                regexInitialiser += "[a-z]{1}";
            }
            else if (isalpha(pszInput[j])) {
                regexInitialiser += pszInput[j];
            }
            else {
                cout << "Invalid character in input string" << endl;
                free(pszDictionary);
                exit(-1);
            }
        }

        regexInitialiser += ")$";

        cout << "Regex string '" << regexInitialiser << "'" << endl;

        char * s = pszDictionary;

        rtn = regcomp(&regex, regexInitialiser.c_str(), REG_EXTENDED | REG_NEWLINE);

        if (rtn) {
            regerror(rtn, &regex, szErrorString, 128);

            cout << "Failed to compile regular expression '" << regexInitialiser << "': " << szErrorString;
            free(pszDictionary);
            exit(-1);
        }

        cout << "Matches for '" << pszInput << "':" << endl;

        while (!regexec(&regex, s, 1, pmatch, 0)) {
            matchOffset = pmatch[0].rm_so;
            matchLength = pmatch[0].rm_eo - pmatch[0].rm_so;

            if (matchLength > REGEX_MATCH_LEN) {
                cout << "*** Match longer than buffer, skipping..." << endl;
            }
            else {
                strncpy(szMatch, &s[matchOffset], matchLength);
                szMatch[matchLength] = 0;

                cout << szMatch << endl;
            }

            s += pmatch[0].rm_eo;
        }

        regfree(&regex);
    }
    else if (mode == MODE_ANAGRAM) {
        /*
        ** Start by building a frequency distribution of the
        ** characters in our input string...
        */
        int     freqArray[26];

        memset(freqArray, 0, sizeof(int) * 26);

        for (int i = 0;i < inputLen;i++) {
            int index = pszInput[i] - 'a';
            freqArray[index]++;
        }

        const char * pszFormatStr = "^([%s]{%d})$";
        
        char * pszRegexString = (char *)malloc(strlen(pszFormatStr) + inputLen + 32); 

        cout << "Anagrams for '" << pszInput << "':" << endl;

        for (int i = inputLen;i >= 1;i--) {
            sprintf(pszRegexString, pszFormatStr, pszInput, i);

            char * s = pszDictionary;

            rtn = regcomp(&regex, pszRegexString, REG_EXTENDED | REG_NEWLINE);

            if (rtn) {
                regerror(rtn, &regex, szErrorString, 128);

                cout << "Failed to compile regular expression '" << pszRegexString << "': " << szErrorString;
                free(pszDictionary);
                exit(-1);
            }

            while (!regexec(&regex, s, 1, pmatch, 0)) {
                matchOffset = pmatch[0].rm_so;
                matchLength = pmatch[0].rm_eo - pmatch[0].rm_so;

                if (matchLength > REGEX_MATCH_LEN) {
                    cout << "*** Match longer than buffer, skipping..." << endl;
                }
                else {
                    strncpy(szMatch, &s[matchOffset], matchLength);
                    szMatch[matchLength] = 0;

//                    cout << "Match: '" << szMatch << "' " << strlen(szMatch) << endl;

                    int     matchFreqArray[26];
                    bool    includeMatch = true;

                    memset(matchFreqArray, 0, sizeof(int) * 26);

                    for (int j = 0;j < (int)strlen(szMatch);j++) {
                        int index = szMatch[j] - 'a';
                        matchFreqArray[index]++;

                        if (matchFreqArray[index] > freqArray[index]) {
                            includeMatch = false;
                        }
                    }

                    if (includeMatch) {
                        cout << szMatch << endl;
                    }
                }

                s += pmatch[0].rm_eo;
            }

            regfree(&regex);
        }

        free(pszRegexString);
    }
    else {
        cout << "Invalid mode: " << mode << endl << endl;
        printUsage();
        free(pszDictionary);
        exit(-1);
    }

    free(pszDictionary);

    return 0;
}
