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
    std::string pickRandomWord() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, wordList.size() - 1);
        return wordList[dist(gen)];
    }

    std::string getUserGuess(int attempt) {
        std::string guess;
        while (true) {
            std::cout << "Attempt " << attempt << "/" << maxAttempts << " (Enter " << wordLength << "-letter word): " << std::endl;
            std::cin >> guess;

            if (guess.size() == wordLength && isalphaString(guess)) {
                std::transform(guess.begin(), guess.end(), guess.begin(), ::tolower);
                return guess;
            }

            std::cout << "Invalid input. Enter a " << wordLength << "-letter word." << std::endl;
        }
    }

    bool isalphaString(const std::string& s) {
        return std::all_of(s.begin(), s.end(), ::isalpha);
    }

    std::vector<LetterState> evaluateGuess(const std::string& guess) {
        std::vector<LetterState> result(wordLength, LetterState::Absent);
        std::string temp = targetWord;

        for (int i = 0; i < wordLength; i++) {
            if (guess[i] == temp[i]) {
                result[i] = LetterState::Correct;
                temp[i] = '*';
            }
        }

        for (int i = 0; i < wordLength; i++) {
            if (result[i] == LetterState::Correct) continue;
            auto pos = temp.find(guess[i]);
            if (pos != std::string::npos) {
                result[i] = LetterState::Present;
                temp[pos] = '*';
            }
        }

        return result;
    }
    void printResult(const std::string& guess, const std::vector<LetterState>& result) {
        for (int i = 0; i < wordLength; i++) {
            if (result[i] == LetterState::Correct) {
                std::cout << "\033[42m\033[97m " << guess[i] << " \033[0m";
            }
            else if (result[i] == LetterState::Present) {
                std::cout << "\033[43m\033[97m " << guess[i] << " \033[0m";
            }
            else {
                std::cout << "\033[100m\033[97m " << guess[i] << " \033[0m";
            }
        }
        std::cout << std::endl;
    }
};
int main() {
    std::vector<std::string> dictionary = {
        "apple", "brave", "cable", "trace", "light", "stone", "sound", "water", "plane", "point",
        "grape", "fruit", "house", "chair", "table", "dream", "quick", "watch", "party", "smile",
        "ten", "cat", "dog", "sun",
        "jinx", "four", "word",
        "sassy", "occur",
        "abduct", "bright", "planet",
    };

    int choice;
    bool running = true;
    while (running) {
        std::cout << "=================================================================" << std::endl;
        std::cout << "|                          WORDLE MENU                          |" << std::endl;
        std::cout << "=================================================================" << std::endl;
        std::cout << "1. Play Classic Wordle (5-letter, 6 attempts, Duplicates OK)" << std::endl;
        std::cout << "2. Play 4-Letter Hard Mode (7 attempts, No duplicates in target)" << std::endl;
        std::cout << "3. Play 6-Letter Challenge (6 attempts)" << std::endl;
        std::cout << "4. Exit Game" << std::endl;
        std::cout << "-----------------------------------------------------------------" << std::endl;
        std::cout << "Enter your choice (1-4): " << std::endl;

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            choice = 0;
        }
        switch (choice) {
        case 1: {
            Wordle game(dictionary);
            game.run();
            break;
        }
        case 2: {
            Wordle game(dictionary, 4, 7, false);
            game.run();
            break;
        }
        case 3: {
            Wordle game(dictionary, 6);
            game.run();
            break;
        }
        case 4: {
            running = false;
            std::cout << "Exiting Wordle Game. Goodbye!" << std::endl;
            break;
        }
        default: {
            std::cout << "Invalid choice. Please enter a number between 1 and 4." << std::endl;
            break;
        }
        }
        std::cout << std::endl;
    }

    return 0;
}