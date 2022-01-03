#include <iostream>
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "simple_regex.h"
#include "xword_error.h"

using namespace std;

#define MODE_XWORD                  1
#define MODE_ANAGRAM                2
#define MIN_ANAGRAM_SOLUTION_LEN    3
#define COLUMN_SPACING              25
#define NUM_DISPLAY_COLUMNS         3

#define DICTIONARY_FILE             "./dictionary.db"


void inline printUsage() {
    cout << "Usage:" << endl;
    cout << "    xcode -d [dictionary file] -xa [input string]" << endl << endl;
    cout << "    -x crossword mode, list all possible matches for input string" << endl;
    cout << "    -a anagram mode, list all anagrams for the word in input string" << endl << endl;
}

char * readDictionary(char * pszDictionaryFile)
{
    char *          pszDictionary;
    FILE *          fptr;
    uint32_t        dictionaryLen;
    
    fptr = fopen(pszDictionaryFile, "rt");

    if (fptr == NULL) {
        throw xword_error(xword_error::buildMsg("Failed to open dictionary file '%s'", pszDictionaryFile));
    }

    fseek(fptr, 0L, SEEK_END);
    dictionaryLen = ftell(fptr);
    fseek(fptr, 0L, SEEK_SET);

    pszDictionary = (char *)malloc(dictionaryLen);

    if (pszDictionary == NULL) {
        fclose(fptr);
        throw xword_error("Failed to allocate memory for dictionary");
    }

    uint32_t bytesRead = (uint32_t)fread(pszDictionary, 1, dictionaryLen, fptr);

    if (bytesRead < dictionaryLen) {
        throw xword_error(xword_error::buildMsg("Failed to read in whole dictionary, got %u of %u bytes", bytesRead, dictionaryLen));
    }

    fclose(fptr);

    return pszDictionary;
}

vector<string> * solveForCrossword(char * pszInput, char * pszDictionary)
{
    int                 inputLen;

    inputLen = (int)strlen(pszInput);

    string regexInitialiser = "^("; 

    for (int i = 0;i < inputLen;i++) {
        if (pszInput[i] == '?') {
            regexInitialiser += "[a-z]{1}";
        }
        else if (isalpha(pszInput[i])) {
            regexInitialiser += pszInput[i];
        }
        else {
            throw xword_error("Invalid character in input string");
        }
    }

    regexInitialiser += ")$";

    SimpleRegex r(regexInitialiser, pszDictionary);

    vector<string> * solutions = new vector<string>();

    while (r.hasMoreMatches()) {
        solutions->push_back(*(r.nextMatch()));
    }

    return solutions;
}

vector<string> * solveForAnagram(char * pszInput, char * pszDictionary)
{
    int             inputLen;
    int             index;
    int             freqArray[26];
    int             matchFreqArray[26];
    bool            includeMatch = true;
    const char *    pszMatch;

    inputLen = (int)strlen(pszInput);

    /*
    ** Start by building a frequency distribution of the
    ** characters in our input string...
    */
    memset(freqArray, 0, sizeof(int) * 26);

    for (int i = 0;i < inputLen;i++) {
        int index = pszInput[i] - 'a';
        freqArray[index]++;
    }

    const char * pszFormatStr = "^([%s]{%d})$";
    
    char * pszRegexString = (char *)malloc(strlen(pszFormatStr) + inputLen + 32);

    if (pszRegexString == NULL) {
        throw xword_error("Failed to allocate memory for regex string");
    }

    vector<string> * solutions = new vector<string>();

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
            includeMatch = true;

            memset(matchFreqArray, 0, sizeof(int) * 26);

            pszMatch = r->nextMatch()->c_str();

            /*
            ** Build a frequency array and compare against the frequency array
            ** for the input string, reject any solution that has too many of
            ** a particular character from the solution. E.g. for the input
            ** 'cabbage', regex will find 'babbage' as a match in the dictionary.
            */
            for (int j = 0;j < (int)strlen(pszMatch);j++) {
                index = pszMatch[j] - 'a';
                matchFreqArray[index]++;

                /*
                ** Reject match if we have too many of a particular character...
                */
                if (matchFreqArray[index] > freqArray[index]) {
                    includeMatch = false;
                }
            }

            if (includeMatch) {
                solutions->push_back(pszMatch);
            }
        }

        delete r;
    }

    free(pszRegexString);

    return solutions;
}

int main(int argc, char *argv[])
{
    int             mode = 0;
    int             inputLen;
    int             solutionLength;
    int             rtn = 0;
    char *          pszInput = NULL;
    char *          pszDictionary;
    char *          pszDictionaryFile = NULL;

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

    try {
        /*
        ** Read in the dictionary...
        */
        pszDictionary = readDictionary(pszDictionaryFile);

        vector<string> *    solutions;
        string              solution;

        if (mode == MODE_XWORD) {
            cout << "Matches for '" << pszInput << "':" << endl;

            solutions = solveForCrossword(pszInput, pszDictionary);
        }
        else if (mode == MODE_ANAGRAM) {
            cout << "Anagrams for '" << pszInput << "':" << endl;

            solutions = solveForAnagram(pszInput, pszDictionary);
        }
        else {
            cout << "Invalid mode: " << mode << endl << endl;
            printUsage();
            rtn = -1;
        }

        int columnNum = 0;

        if (solutions != NULL) {
            for (int i = 0;i < (int)solutions->size();i++) {
                solution = solutions->at(i);
                solutionLength = solution.length();

                cout << solution;

                if (columnNum < (NUM_DISPLAY_COLUMNS - 1)) {
                    for (int j = 0;j < (COLUMN_SPACING - solutionLength);j++) {
                        cout << " ";
                    }

                    columnNum++;
                }
                else {
                    cout << endl;
                    columnNum = 0;
                }
            }

            cout << endl;
            
            delete solutions;
        }

        free(pszDictionaryFile);
        free(pszInput);
        free(pszDictionary);
    }
    catch (xword_error & xe) {
        cout << "Caught exception: " << xe.what() << endl;
    }
    catch (regex_error & re) {
        cout << "Caught exception: " << re.what() << endl;
    }

    return rtn;
}
