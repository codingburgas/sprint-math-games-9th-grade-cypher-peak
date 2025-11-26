#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <set>
#include <limits>
#include <cstdlib>
#include <chrono>
#include <thread>

void clearScreen() {
    std::system("cls");
}

void printBanner() {
    std::cout << "\033[32m";
    std::cout << "=================================================================\n";
    std::cout << R"(
$$\      $$\  $$$$$$\  $$$$$$$\  $$$$$$$\  $$\       $$$$$$$$\
$$ | $\  $$ |$$  __$$\ $$  __$$\ $$  __$$\ $$ |      $$  _____|
$$ |$$$\ $$ |$$ /  $$ |$$ |  $$ |$$ |  $$ |$$ |      $$ |      
$$ $$ $$\$$ |$$ |  $$ |$$$$$$$  |$$ |  $$ |$$ |      $$$$$\    
$$$$  _$$$$ |$$ |  $$ |$$  __$$< $$ |  $$ |$$ |      $$  __|   
$$$  / \$$$ |$$ |  $$ |$$ |  $$ |$$ |  $$ |$$ |      $$ |      
$$  /   \$$ | $$$$$$  |$$ |  $$ |$$$$$$$  |$$$$$$$$\ $$$$$$$$\
\__/     \__| \______/ \__|  \__|\_______/ \________|\________|
)" << std::endl;
    std::cout << "=================================================================\n";
    std::cout << "\033[0m";
}

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
                << " found in the dictionary." << std::endl;
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

        printBanner();
        std::cout << "* Current Settings:\n";
        std::cout << "* Word Length: " << wordLength << "\n";
        std::cout << "* Max Attempts: " << maxAttempts << "\n";
        std::cout << "* Target Word Allows Duplicate Letters: " << (allowDuplicatesInTarget ? "Yes" : "No") << "\n";
        std::cout << "* Dictionary Size: " << wordList.size() << " Words\n";
        std::cout << "-----------------------------------------------------------------\n\n";
        std::cout << "You have " << maxAttempts << " Attempts to Guess The " << wordLength << "-Letter Word.\n";

        for (int attempt = 1; attempt <= maxAttempts; attempt++) {

            std::string guess = getUserGuess(attempt);

            if (guess.empty()) {

                continue;
            }

            auto result = evaluateGuess(guess);
            printResult(guess, result);

            if (guess == targetWord) {
                std::cout << "\n\033[92m Correct! The Word Was: \033[93m" << targetWord << "\033[0m\n";
                return;
            }
        }

        std::cout << "\n\033[92m Out of Attempts! The Word Was: \033[93m" << targetWord << "\033[0m\n";
    }

private:
    enum class LetterState { Correct, Present, Absent };

    std::vector<std::string> wordList;
    std::string targetWord;
    int maxAttempts;
    int wordLength;
    bool allowDuplicatesInTarget;

    int guessTimeLimit = 30;

    bool hasDuplicateLetters(const std::string& word) const {
        std::set<char> uniqueChars;
        for (char c : word) {
            if (uniqueChars.count(c)) return true;
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
            if (isalphaString(lowerWord)) filteredList.push_back(lowerWord);
        }
        return filteredList;
    }

    std::string pickRandomWord() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, wordList.size() - 1);
        return wordList[dist(gen)];
    }


    bool runGuessTimer(bool& expiredFlag) {
        using namespace std::chrono;

        auto start = steady_clock::now();
        int lastShown = -1;

        while (!expiredFlag) {
            auto now = steady_clock::now();
            int elapsed = duration_cast<seconds>(now - start).count();
            int remaining = guessTimeLimit - elapsed;

            if (remaining != lastShown && remaining >= 0) {
                std::cout << "\r Time Left: " << remaining << "s  " << std::flush;
                lastShown = remaining;
            }

            if (remaining <= 0) {
                std::cout << "\n Time is up!\n";
                expiredFlag = true;
                return false;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return false;
    }

    std::string getUserGuess(int attempt) {
        std::string guess;
        bool timerExpired = false;

        std::cout << "\nAttempt " << attempt << "/" << maxAttempts
            << " (Enter " << wordLength << "-Letter Word, "
            << guessTimeLimit << "s Timer)\n";

        std::thread timerThread([&]() {
            runGuessTimer(timerExpired);
            });

        std::cin >> guess;
        timerExpired = true;
        timerThread.join();

        if (guess.size() == 0 || timerExpired == true && guess.size() != wordLength) {
            std::cout << "❌ Attempt failed due to timeout.\n";
            return "";
        }

        if (guess.size() == wordLength && isalphaString(guess)) {
            std::transform(guess.begin(), guess.end(), guess.begin(), ::tolower);
            return guess;
        }

        std::cout << "Invalid Input. Enter a " << wordLength << "-Letter Word.\n";
        return getUserGuess(attempt);
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
        std::cout << "\n";
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
        printBanner();
        std::cout << "1. Play Classic Wordle (5-Letter, 6 Attempts)\n";
        std::cout << "2. Play 4-Letter Hard Mode (7 Attempts, No Duplicates)\n";
        std::cout << "3. Play 6-Letter Challenge (6 Attempts)\n";
        std::cout << "4. Exit Game\n";
        std::cout << "-----------------------------------------------------------------\n";
        std::cout << "Enter Your Choice (1-4): ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            choice = 0;
        }

        switch (choice) {
        case 1:
            clearScreen();
            Wordle(dictionary).run();
            break;

        case 2:
            clearScreen();
            Wordle(dictionary, 4, 7, false).run();
            break;

        case 3:
            clearScreen();
            Wordle(dictionary, 6).run();
            break;

        case 4:
            running = false;
            std::cout << "Exiting Wordle Game. Goodbye!\n";
            break;

        default:
            std::cout << "Invalid Choice. Try again.\n";
            break;
        }

        std::cout << "\n";
    }

    return 0;
}