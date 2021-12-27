#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "simple_regex.h"

using namespace std;

#define MODE_XWORD                  1
#define MODE_ANAGRAM                2

#define DICTIONARY_FILE             "./dictionary.db"

#define MIN_ANAGRAM_SOLUTION_LEN    3


void inline printUsage() {
    cout << "Usage:" << endl;
    cout << "    xcode -d [dictionary file] -xa [input string]" << endl << endl;
    cout << "    -x crossword mode, list all possible matches for input string" << endl;
    cout << "    -a anagram mode, list all anagrams for the word in input string" << endl << endl;
}

int main(int argc, char *argv[])
{
    int             mode = 0;
    int             inputLen;
    char *          pszInput = NULL;
    char *          pszDictionary;
    char *          pszDictionaryFile = NULL;
    FILE *          fptr;
    uint32_t        dictionaryLen;

    if (argc > 2) {
        for (int i = 1;i < argc;i++) {
            if (argv[i][0] == '-') {
                if (argv[i][1] == 'x') {
                    mode = MODE_XWORD;

                    if (i < argc - 1) {
                        pszInput = strdup(&argv[i+1][0]);
                    }
                }
                else if (argv[i][1] == 'a') {
                    mode = MODE_ANAGRAM;

                    if (i < argc - 1) {
                        pszInput = strdup(&argv[i+1][0]);
                    }
                }
                else if (argv[i][1] == 'd') {
                    pszDictionaryFile = strdup(&argv[i+1][0]);
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

    if (pszInput == NULL) {
        cout << "No input string supplied..." << endl;
        exit(-1);
    }

    inputLen = (int)strlen(pszInput);

    /*
    ** Make sure the input string is lower case...
    */
    for (int i = 0;i < inputLen;i++) {
        pszInput[i] = tolower(pszInput[i]);
    }

    if (pszDictionaryFile == NULL) {
        pszDictionaryFile = strdup(DICTIONARY_FILE);
    }

    fptr = fopen(pszDictionaryFile, "rt");

    if (fptr == NULL) {
        cout << "Failed to open dictionary file '" << pszDictionaryFile << "'" << endl;
        free(pszDictionaryFile);
        free(pszInput);
        exit(-1);
    }

    free(pszDictionaryFile);

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

        SimpleRegex r(regexInitialiser, pszDictionary);

        cout << "Matches for '" << pszInput << "':" << endl;

        while (r.hasMoreMatches()) {
            cout << *(r.nextMatch()) << endl;
        }

        free(pszInput);
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

        for (int i = inputLen;i >= MIN_ANAGRAM_SOLUTION_LEN;i--) {
            /*
            ** Build the regex string...
            ** Starting with solutions the length of the input string
            ** find solutions down to MIN_ANAGRAM_SOLUTION_LEN
            */
            sprintf(pszRegexString, pszFormatStr, pszInput, i);

            SimpleRegex * r = new SimpleRegex(string(pszRegexString), pszDictionary);

            /*
            ** Find matches...
            */
            while (r->hasMoreMatches()) {
                int             matchFreqArray[26];
                bool            includeMatch = true;
                const char *    pszMatch;

                memset(matchFreqArray, 0, sizeof(int) * 26);

                pszMatch = r->nextMatch()->c_str();

                /*
                ** Build a frequency array and compare against the frequency array
                ** for the input string, reject any solution that has too many of
                ** a particular character from the solution. E.g. for the input
                ** 'cabbage', regex will find 'babbage' as a match in the dictionary.
                */
                for (int j = 0;j < (int)strlen(pszMatch);j++) {
                    int index = pszMatch[j] - 'a';
                    matchFreqArray[index]++;

                    /*
                    ** Reject match if we have too many of a particular character...
                    */
                    if (matchFreqArray[index] > freqArray[index]) {
                        includeMatch = false;
                    }
                }

                if (includeMatch) {
                    cout << pszMatch << endl;
                }
            }

            delete r;
        }

        free(pszRegexString);
        free(pszInput);
    }
    else {
        cout << "Invalid mode: " << mode << endl << endl;
        printUsage();
        free(pszInput);
        free(pszDictionary);
        exit(-1);
    }

    free(pszDictionary);

    return 0;
}
