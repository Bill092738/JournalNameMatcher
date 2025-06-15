//run with this command for best performance:
//g++ -O3 -march=native -funroll-loops -flto -s -pipe -o normalizer normalizer.cpp

#include <fstream>
#include <string>
#include <cctype>

int main() {
    std::ifstream inFile("preinput.csv");
    std::ofstream outFile("input.csv");

    if (!inFile.is_open() || !outFile.is_open()) {
        return 1;
    }

    std::string line;
    bool isFirstLine = true;

    while (std::getline(inFile, line)) {
        if (isFirstLine) {
            outFile << line << '\n';
            isFirstLine = false;
        } else {
            if (line.find('"') == std::string::npos) {
                outFile << '"' << line << '"' << '\n';
            } else {
                outFile << '"';
                for (char c : line) {
                    if (c == '"') {
                        outFile << "\"\"";
                    } else {
                        outFile << c;
                    }
                }
                outFile << '"' << '\n';
            }
        }
    }

    inFile.close();
    outFile.close();
    return 0;
}