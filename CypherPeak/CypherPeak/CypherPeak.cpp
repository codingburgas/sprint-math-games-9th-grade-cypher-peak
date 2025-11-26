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
#include <cctype>

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
)" << "\n";
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
            std::cerr << "Error: No valid words of length " << wordLength << " found.\n";
            targetWord = "";
        }
        else {
            targetWord = pickRandomWord();
        }
    }

    bool run(int& attemptsUsed) {
        if (targetWord.empty()) return false;

        printBanner();
        std::cout << "* Word Length: " << wordLength << " | Max Attempts: " << maxAttempts << "\n";
        std::cout << "You have " << maxAttempts << " attempts to guess the word.\n";

        attemptsUsed = 0;

        for (int attempt = 1; attempt <= maxAttempts; ++attempt) {
            ++attemptsUsed;
            std::string guess = getUserGuess(attempt);

            if (guess.empty()) continue;

            auto result = evaluateGuess(guess);
            printResult(guess, result);

            if (guess == targetWord) {
                std::cout << "\nCorrect! The Word Was: " << targetWord << "\n";
                return true;
            }
        }

        std::cout << "\nOut of attempts! The Word Was: " << targetWord << "\n";
        return false;
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
        std::vector<std::string> filtered;
        for (const auto& word : words) {
            if (word.length() != wordLength) continue;
            if (!allowDuplicatesInTarget && hasDuplicateLetters(word)) continue;
            std::string w = word;
            std::transform(w.begin(), w.end(), w.begin(), ::tolower);
            if (isalphaString(w)) filtered.push_back(w);
        }
        return filtered;
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
                std::cout << "\rTime Left: " << remaining << "s  " << std::flush;
                lastShown = remaining;
            }
            if (remaining <= 0) {
                std::cout << "\nTime is up!\n";
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
            << " (" << guessTimeLimit << "s Timer)\n";

        std::thread timerThread([&]() { runGuessTimer(timerExpired); });
        std::cin >> guess;
        timerExpired = true;
        timerThread.join();

        if (guess.empty() || (timerExpired && guess.size() != wordLength)) {
            std::cout << "Attempt failed due to timeout.\n";
            return "";
        }

        if (guess.size() == wordLength && isalphaString(guess)) {
            std::transform(guess.begin(), guess.end(), guess.begin(), ::tolower);
            return guess;
        }

        std::cout << "Invalid input. Enter a " << wordLength << "-letter word.\n";
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
            if (result[i] == LetterState::Correct)
                std::cout << "\033[42m\033[97m " << guess[i] << " \033[0m";
            else if (result[i] == LetterState::Present)
                std::cout << "\033[43m\033[97m " << guess[i] << " \033[0m";
            else
                std::cout << "\033[100m\033[97m " << guess[i] << " \033[0m";
        }
        std::cout << "\n";
    }
};

int main() {
    std::vector<std::string> dictionary = {
        "apple","brave","cable","trace","light","stone","sound","water","plane","point",
        "grape","fruit","house","chair","table","dream","quick","watch","party","smile",
        "jinx","four","word","fizz","bolt","maze","clip","haze",
        "sassy","occur","abduct","bright","planet"
    };

    int choice;
    bool running = true;

    int totalWins = 0, totalPoints = 0, totalLosses = 0, currentStreak = 0;

    while (running) {
        printBanner();
        std::cout << "Score: Wins=" << totalWins << " | Losses=" << totalLosses
            << " | Streak=" << currentStreak << " | Points=" << totalPoints << "\n";
        std::cout << "-----------------------------------------------------------------\n";
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

        Wordle game(dictionary);
        int maxAttempts = 6;

        switch (choice) {
        case 1: clearScreen(); game = Wordle(dictionary, 5, 6, true); maxAttempts = 6; break;
        case 2: clearScreen(); game = Wordle(dictionary, 4, 7, false); maxAttempts = 7; break;
        case 3: clearScreen(); game = Wordle(dictionary, 6, 6, true); maxAttempts = 6; break;
        case 4: running = false; std::cout << "Exiting Wordle. Goodbye!\n"; continue;
        default: std::cout << "Invalid choice. Try again.\n"; continue;
        }

        int attemptsUsed = 0;
        bool won = game.run(attemptsUsed);

        if (won) {
            totalWins++;
            currentStreak++;
            int pointsEarned = (maxAttempts - attemptsUsed + 1) * 10;
            totalPoints += pointsEarned;
            std::cout << "You earned " << pointsEarned << " points!\n";
        }
        else {
            totalLosses++;
            currentStreak = 0;
        }

        std::cout << "Current Score: Wins=" << totalWins
            << " | Losses=" << totalLosses
            << " | Streak=" << currentStreak
            << " | Total Points=" << totalPoints << "\n\n";
    }

    return 0;
}
