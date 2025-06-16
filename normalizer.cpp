//run with this command for best performance:
//g++ -O3 -march=native -funroll-loops -flto -s -pipe -o normalizer normalizer.cpp

#include <algorithm>
#include <fstream>
#include <string>
#include <cctype>
#include <unordered_map>

std::string toLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return lowerStr;
}

int main() {
    std::ifstream inFile("preinput.csv");
    std::ofstream outFile("input.csv");
    std::ofstream tempFile("temp.csv");

    if (!inFile.is_open() || !outFile.is_open() || !tempFile.is_open()) {
        return 1;
    }

    std::unordered_map<std::string, int> duplicateCounter;
    std::unordered_map<std::string, std::string> originalLineMap;
    std::string line;
    bool isFirstLine = true;

    // First pass: count duplicates (case-insensitive)
    while (std::getline(inFile, line)) {
        if (!isFirstLine) {
            std::string lowerLine = toLower(line);
            duplicateCounter[lowerLine]++;
            // Store the first original line for output
            if (originalLineMap.find(lowerLine) == originalLineMap.end()) {
                originalLineMap[lowerLine] = line;
            }
        } else {
            isFirstLine = false;
        }
    }

    // Reset file pointer to beginning
    inFile.clear();
    inFile.seekg(0);
    isFirstLine = true;

    // Write header to both files
    tempFile << "abbreviation,count\n";
    
    // Second pass: write unique entries and log duplicates
    while (std::getline(inFile, line)) {
        if (isFirstLine) {
            outFile << line << '\n';
            isFirstLine = false;
            continue;
        }

        std::string lowerLine = toLower(line);

        // Only write to output file if this is the first occurrence
        if (duplicateCounter[lowerLine] > 0) {
            std::string outputLine;
            std::string original = originalLineMap[lowerLine];
            if (original.find('"') == std::string::npos) {
                outputLine = '"' + original + '"';
            } else {
                outputLine = "\"";
                for (char c : original) {
                    if (c == '"') {
                        outputLine += "\"\"";
                    } else {
                        outputLine += c;
                    }
                }
                outputLine += '"';
            }

            // Write to output file only once
            outFile << outputLine << '\n';
            
            // Log to temp file if there were duplicates
            if (duplicateCounter[lowerLine] > 1) {
                tempFile << outputLine << "," << duplicateCounter[lowerLine] << '\n';
            }
            
            // Mark as processed by setting count to 0
            duplicateCounter[lowerLine] = 0;
        }
    }

    inFile.close();
    outFile.close();
    tempFile.close();
    return 0;
}