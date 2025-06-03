#include <string>
#include <cctype>
#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <fstream>
#include <sstream>

// Returns the union of characters from str2 that are present in str1 (case-insensitive, preserves order and repeats up to the count in str1)
std::string takeUnion(const std::string& str1, const std::string& str2) {
    std::string result;
    std::unordered_map<char, int> char_count;

    // Count occurrences of each character in str1 (case-insensitive)
    for (char c : str1) {
        char_count[std::tolower(static_cast<unsigned char>(c))]++;
    }

    // For each character in str2, if it exists in str1 (by count), add to result and decrement count
    for (char c : str2) {
        char lower_c = std::tolower(static_cast<unsigned char>(c));
        if (char_count[lower_c] > 0) {
            result += c;
            char_count[lower_c]--;
        }
    }
    return result;
}

float toOwnSimilarity(const std::string& str1, const std::string& str2) {
    std::string toUnionResult = takeUnion(str1, str2);
    if (str1.empty()) return 0.0f;
    // Count only the characters from str1 that were matched (case-insensitive)
    // toUnionResult.size() / str1.size() * 100
    float percentage = (static_cast<float>(toUnionResult.size()) / static_cast<float>(str1.size())) * 100.0f;
    return percentage;
}

std::string NormalizeStr(const std::string& str) {
    std::string normalized;
    // This function will remove all non-alphanumeric and non-number characters and convert to upercase
    for (char c : str) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            normalized += std::toupper(static_cast<unsigned char>(c));
        }
    }
    return normalized;
}

int editDistance(const std::string& str1, const std::string& str2) {
    int len1 = str1.size();
    int len2 = str2.size();
    if (len1 == 0) return len2;
    if (len2 == 0) return len1;

    // Create a distance matrix
    std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1));

    for (int i = 0; i <= len1; ++i) dp[i][0] = i;
    for (int j = 0; j <= len2; ++j) dp[0][j] = j;

    for (int i = 1; i <= len1; ++i) {
        for (int j = 1; j <= len2; ++j) {
            int cost = (str1[i - 1] == str2[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost});
        }
    }
    return dp[len1][len2];
}

float editDistanceSimilarity(const std::string& str1, const std::string& str2) {
    // This function calculates the edit distance similarity percentage
    std::string emptyStr = "";
    // Edit distance to create str2 from empty
    int Maxdistance = editDistance(emptyStr, str2);
    // Edit distance to convert str1 to str2
    int distance = editDistance(str1, str2);
    if (Maxdistance == 0) return 100.0f; // If str2 is empty, similarity is 100%
    // Calculate similarity percentage
    float similarity = (1.0f - static_cast<float>(distance) / Maxdistance) * 100.0f;
    return similarity;
}

int longestCommonStrLength(const std::string& str1, const std::string& str2) {
    int len1 = str1.size();
    int len2 = str2.size();
    if (len1 == 0 || len2 == 0) return 0;

    // Create a matrix to store lengths of longest common suffixes
    std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1, 0));
    int maxLength = 0;

    for (int i = 1; i <= len1; ++i) {
        for (int j = 1; j <= len2; ++j) {
            if (str1[i - 1] == str2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
                maxLength = std::max(maxLength, dp[i][j]);
            }
        }
    }
    return maxLength;
}

int secondLongestCommonStrLength(const std::string& str1, const std::string& str2) {
    // This function calculates the second longest common substring length
    int len1 = str1.size();
    int len2 = str2.size();
    if (len1 == 0 || len2 == 0) return 0;

    // Create a matrix to store lengths of longest common suffixes
    std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1, 0));
    int maxLength = 0;
    int secondMaxLength = 0;

    for (int i = 1; i <= len1; ++i) {
        for (int j = 1; j <= len2; ++j) {
            if (str1[i - 1] == str2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
                if (dp[i][j] > maxLength) {
                    secondMaxLength = maxLength;
                    maxLength = dp[i][j];
                } else if (dp[i][j] > secondMaxLength && dp[i][j] < maxLength) {
                    secondMaxLength = dp[i][j];
                }
            }
        }
    }
    return secondMaxLength;
}

float LCSSimilarity(const std::string& str1, const std::string& str2) {
    // This function calculates the longest common substring similarity percentage
    int lcsLength = longestCommonStrLength(str1, str2);
    if (lcsLength == 0) return 0.0f; // If no common substring, similarity is 0%
    // Calculate similarity percentage based on the length of the longest common substring
    float similarity = (static_cast<float>(lcsLength) / std::max(str1.size(), str2.size())) * 100.0f;
    return similarity;
}

float secondLCSSimilarity(const std::string& str1, const std::string& str2) {
    // This function calculates the second longest common substring similarity percentage
    int secondLcsLength = secondLongestCommonStrLength(str1, str2);
    if (secondLcsLength == 0) return 0.0f; // If no second common substring, similarity is 0%
    // Calculate similarity percentage based on the length of the second longest common substring
    float similarity = (static_cast<float>(secondLcsLength) / std::max(str1.size(), str2.size())) * 100.0f;
    return similarity;
}

float MixedSimilarity(const std::string& str1, const std::string& str2) {
    // This function calculates the mixed similarity percentage
    float halfJaccard = toOwnSimilarity(str1, str2);
    float editDistSim = editDistanceSimilarity(str1, str2);
    float CSSim = LCSSimilarity(str1, str2) + secondLCSSimilarity(str1, str2);

    // Combine the similarities (you can adjust the weights as needed)
    return (halfJaccard + editDistSim + CSSim) / 3.0f;
}

// Helper function to trim whitespace from both ends of a string
inline std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

// Enhanced CSV reader: handles quoted fields, commas, and trims whitespace
std::vector<std::vector<std::string>> readCSV(const std::string& filename) {
    std::vector<std::vector<std::string>> table;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open " << filename << " for reading.\n";
        return table;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> row;
        std::istringstream ss(line);
        std::string field;
        bool in_quotes = false;
        char c;
        std::string current;

        while (ss.get(c)) {
            if (c == '"' && (current.empty() || in_quotes)) {
                in_quotes = !in_quotes;
                if (in_quotes && ss.peek() == '"') { // handle escaped quotes
                    current += '"';
                    ss.get();
                }
            } else if (c == ',' && !in_quotes) {
                row.push_back(trim(current));
                current.clear();
            } else {
                current += c;
            }
        }
        row.push_back(trim(current));
        table.push_back(row);
    }
    return table;
}

void writeCSV(const std::string& filename, const std::vector<std::vector<std::string>>& data) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open " << filename << " for writing.\n";
        return;
    }
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            // Add quotes if field contains comma or quote
            if (row[i].find(',') != std::string::npos || row[i].find('"') != std::string::npos) {
                file << '"';
                for (char c : row[i]) {
                    if (c == '"') file << "\"\""; // escape quotes
                    else file << c;
                }
                file << '"';
            } else {
                file << row[i];
            }
            if (i != row.size() - 1) file << ",";
        }
        file << "\n";
    }
}

void print_usage() {
    std::cout << "============================================\n";
    std::cout << " Journal Similarity Calculator\n";
    std::cout << "============================================\n";
    std::cout << "Usage:\n";
    std::cout << "  1. Place your input CSV as 'output.csv' in the current directory.\n";
    std::cout << "  2. Run this program. It will generate 'output_with_similarity.csv' with similarity scores.\n";
    std::cout << "\n";
    std::cout << "Output:\n";
    std::cout << "  output_with_similarity.csv: Each row contains the original pair and the similarity score.\n";
    std::cout << "\n";
    std::cout << "Author: Haoran Zhao\n";
    std::cout << "Contact: bill092738@gmail.com\n";
    std::cout << "============================================\n\n";
}

int main() {
    print_usage();

    auto data = readCSV("output.csv");
    // Add header for new column if present
    if (!data.empty() && data[0].size() >= 2) {
        if (data[0].size() == 2) {
            data[0].push_back("similarity");
        } else if (data[0].size() > 2) {
            data[0][2] = "similarity";
        }
    }
    // Process each row (skip header)
    size_t total = data.size() > 1 ? data.size() - 1 : 0; // exclude header
    size_t barWidth = 40;
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i].size() < 2) continue;
        std::string s1 = NormalizeStr(data[i][0]);
        std::string s2 = NormalizeStr(data[i][1]);
        float sim = MixedSimilarity(s1, s2);
        if (sim > 100.0f) sim = 100.0f;
        else if (sim < 0.0f) sim = 0.0f;
        std::ostringstream oss;
        oss.precision(2);
        oss << std::fixed << sim;
        if (data[i].size() < 3) {
            data[i].push_back(oss.str());
        } else {
            data[i][2] = oss.str();
        }

        // Loading bar
        float progress = total ? static_cast<float>(i) / total : 1.0f;
        int pos = static_cast<int>(barWidth * progress);
        std::cout << "\r[";
        for (size_t j = 0; j < barWidth; ++j) {
            if (j < pos) std::cout << "=";
            else if (j == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(progress * 100) << "%";
        std::cout.flush();
    }
    std::cout << std::endl;
    writeCSV("output_with_similarity.csv", data);
    std::cout << "Similarity calculation finished. Output written to output_with_similarity.csv" << std::endl;
    return 0;
}