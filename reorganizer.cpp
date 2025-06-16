#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// Helper to trim quotes
std::string trim_quotes(const std::string& s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

int main() {
    std::unordered_map<std::string, int> counts;
    std::ifstream tempFile("temp.csv");
    std::string line;

    // Skip header
    std::getline(tempFile, line);

    // Read temp.csv: abbreviation,count
    while (std::getline(tempFile, line)) {
        size_t last_quote = line.rfind('"');
        if (last_quote != std::string::npos) {
            std::string abbr = line.substr(1, last_quote - 1);
            std::string countStr = line.substr(last_quote + 2); // skip comma
            if (!countStr.empty()) {
                counts[abbr] = std::stoi(countStr);
            }
        }
    }
    tempFile.close();

    // Read journal_matches_output.csv
    std::ifstream mapFile("journal_matches_output.csv");
    std::vector<std::pair<std::string, std::string>> mappings;
    // Read header
    std::getline(mapFile, line);

    while (std::getline(mapFile, line)) {
        std::stringstream ss(line);
        std::string abbr, full;
        if (std::getline(ss, abbr, ',') && std::getline(ss, full)) {
            abbr = trim_quotes(abbr);
            full = trim_quotes(full);
            mappings.emplace_back(abbr, full);
        }
    }
    mapFile.close();

    // Write expanded output
    std::ofstream outFile("journal_matches_expanded.csv");
    outFile << "abbreviation,full_name\n";
    for (const auto& pair : mappings) {
        int repeat = counts.count(pair.first) ? counts[pair.first] : 1;
        for (int i = 0; i < repeat; ++i) {
            outFile << '"' << pair.first << "\",\"" << pair.second << "\"\n";
        }
    }
    outFile.close();
    return 0;
}