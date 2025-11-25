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
        for (int attempt = 1; attempt <= maxAttempts; attempt++) {
            std::string guess = getUserGuess(attempt);
            auto result = evaluateGuess(guess);
            printResult(guess, result);

            if (guess == targetWord) {
                std::cout << std::endl;
                std::cout << "🎉 Correct! The word was: " << targetWord << std::endl;
                return;
            }
        }

        std::cout << std::endl;
        std::cout << "❌ Out of attempts! The word was: " << targetWord << std::endl;
    }

private:
    enum class LetterState { Correct, Present, Absent };

    std::vector<std::string> wordList;
    std::string targetWord;
    int maxAttempts;
    int wordLength;
    bool allowDuplicatesInTarget;

    bool hasDuplicateLetters(const std::string& word) const {
        std::set<char> uniqueChars;
        for (char c : word) {
            if (uniqueChars.count(c)) {
                return true;
            }
            uniqueChars.insert(c);
        }
        return false;
    }

    std::vector<std::string> filterAndValidateWordList(const std::vector<std::string>& words) {
        std::vector<std::string> filteredList;
        for (const auto& word : words) {
            if (word.length() != wordLength) continue;

            if (!allowDuplicatesInTarget && hasDuplicateLetters(word)) continue;

            std::string lowerWord = word;
            std::transform(lowerWord.begin(), lowerWord.end(), lowerWord.begin(), ::tolower);
            if (isalphaString(lowerWord) && lowerWord.length() == wordLength) {
                filteredList.push_back(lowerWord);
            }
        }
        return filteredList;
    }