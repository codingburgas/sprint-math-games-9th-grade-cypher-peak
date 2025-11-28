#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

void clearScreen() {
    std::cout << "\033[2J\033[1;1H"; /
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
        wordList = filterWordList(words);
        if (!wordList.empty()) {
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
            attemptsUsed++;
            std::string guess = getUserGuess(attempt);

            if (guess.empty()) continue;

            auto result = evaluateGuess(guess);
            printResult(guess, result);

            if (guess == targetWord) {
                std::cout << "\n \033[32m Correct! The Word Was: " << targetWord << "\033[32m \n";
                return true;
            }
        }

        std::cout << "\n \033[31m Out of attempts! The Word Was: " << targetWord << "\033[31m \n";
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
        bool seen[26] = { false };
        for (char c : word) {
            int idx = c - 'a';
            if (seen[idx]) return true;
            seen[idx] = true;
        }
        return false;
    }

    bool isAlphaString(const std::string& s) {
        for (char c : s) {
            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
                return false;
        }
        return true;
    }

    void lowerString(std::string& s) {
        for (char& c : s)
            if (c >= 'A' && c <= 'Z')
                c += ('a' - 'A');
    }

    std::vector<std::string> filterWordList(const std::vector<std::string>& words) {
        std::vector<std::string> out;
        for (std::string w : words) {
            if (w.length() != wordLength) continue;
            if (!allowDuplicatesInTarget && hasDuplicateLetters(w)) continue;
            lowerString(w);
            if (isAlphaString(w)) out.push_back(w);
        }
        return out;
    }

    std::string pickRandomWord() {
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::mt19937 rng(seed);
        return wordList[rng() % wordList.size()];
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
        std::cout << "\nAttempt " << attempt << "/" << maxAttempts
            << " (" << guessTimeLimit << "s Timer)\n";

        bool expired = false;
        std::string guess;

        std::thread timerThread([&]() { runGuessTimer(expired); });
        std::cin >> guess;
        expired = true;
        timerThread.join();

        if (guess.length() != wordLength || !isAlphaString(guess)) {
            std::cout << "Invalid input.\n";
            return "";
        }

        lowerString(guess);
        return guess;
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
            size_t pos = temp.find(guess[i]);
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
        std::cout << "1. Play Classic Wordle\n";
        std::cout << "2. Play 4-letter Hard Mode\n";
        std::cout << "3. Play 6-letter Challenge\n";
        std::cout << "4. Exit\n";
        std::cout << "-----------------------------------------------------------------\n";
        std::cout << "Enter Choice: ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::string dump;
            std::cin >> dump;
            continue;
        }

        Wordle game(dictionary);
        int maxAttempts = 6;

        switch (choice) {
        case 1: clearScreen(); game = Wordle(dictionary, 5, 6, true); break;
        case 2: clearScreen(); game = Wordle(dictionary, 4, 7, false); break;
        case 3: clearScreen(); game = Wordle(dictionary, 6, 6, true); break;
        case 4: running = false; continue;
        default: continue;
        }

        int attemptsUsed = 0;
        bool won = game.run(attemptsUsed);

        if (won) {
            totalWins++;
            currentStreak++;
            int pts = (maxAttempts - attemptsUsed + 1) * 10;
            totalPoints += pts;
            std::cout << "You earned " << pts << " points!\n";
        }
        else {
            totalLosses++;
            currentStreak = 0;
        }
    }

    return 0;
}
