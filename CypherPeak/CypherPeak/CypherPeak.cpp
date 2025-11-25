#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <set>
#include <limits>

class Wordle {
public:
    Wordle(const std::vector<std::string>& words,
        int wordLength = 5,
        int attempts = 6,
        bool allowDuplicatesInTarget = true)
        : maxAttempts(attempts),
        wordLength(wordLength),
        allowDuplicatesInTarget(allowDuplicatesInTarget)
    {
        wordList = filterAndValidateWordList(words);

        if (wordList.empty()) {
            std::cerr << "Error: No valid words of length " << wordLength
                << " found in the dictionary (or all contain duplicate letters and 'allowDuplicatesInTarget' is false)." << std::endl;
            targetWord = "";
        }
        else {
            targetWord = pickRandomWord();
        }
    }

    void run() {
        if (targetWord.empty()) {
            std::cout << "Cannot start game due to dictionary error." << std::endl;
            return;
        }

        std::cout << "=================================================================" << std::endl;
        std::cout << "|                         C++ WORDLE GAME                       |" << std::endl;
        std::cout << "=================================================================" << std::endl;

        std::cout << std::endl;
        std::cout << "🎯 Current Settings:" << std::endl;
        std::cout << "* Word Length: " << wordLength << std::endl;
        std::cout << "* Max Attempts: " << maxAttempts << std::endl;
        std::cout << "* Target word allows duplicate letters: " << (allowDuplicatesInTarget ? "Yes" : "No (Hard Mode)") << std::endl;
        std::cout << "* Dictionary Size: " << wordList.size() << " words" << std::endl;
        std::cout << "-----------------------------------------------------------------" << std::endl;
        std::cout << std::endl;
        std::cout << "You have " << maxAttempts << " attempts to guess the " << wordLength << "-letter word." << std::endl;
        std::cout << std::endl;

