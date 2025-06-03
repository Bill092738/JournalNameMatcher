#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// =======================
// Constants
// =======================
#define MAX_JOURNALS 10000
#define MAX_NAME_LEN 128
#define COLUMN_COUNT 4
#define MAX_ABBR 5
#define MAX_WORDS 16
#define MAX_INPUT 512
#define MAX_INPUT_ABBR_LEN 128

// =======================
// Structures
// =======================

typedef struct {
    char journals[COLUMN_COUNT][MAX_JOURNALS][MAX_NAME_LEN];
    int count[COLUMN_COUNT];
} JournalMap;

typedef struct {
    const char* word;
    const char* abbr;
} AbbrRule;

typedef struct {
    char* abbr[MAX_ABBR]; // 5 types of abbreviations
    const char* fullname; // pointer to the full journal name
} JournalAbbr;

typedef struct {
    JournalAbbr journals[COLUMN_COUNT][MAX_JOURNALS];
    int count[COLUMN_COUNT];
} JournalAbbrMap;

typedef struct {
    char abbrs[MAX_INPUT][MAX_INPUT_ABBR_LEN];
    int count;
} InputAbbrList;

// =======================
// Function Declarations
// =======================

void readCSV(const char* filename);
void normalizeSingleName(char* name);
void normalizeName();
void generateAbbrs();
void readINPUT(const char* filename);
void cleanAbbr(const char* src, char* dst);
int editDistance(const char* str1, const char* str2);
const char* find_abbr(const char* word);
int split_words(const char* name, char words[MAX_WORDS][MAX_NAME_LEN]);
const char* calculateSimilarity(const char* inputAbbr);
void writeAns(const char* inputFile, const char* outputFile);
static int is_alnum(char c);

// =======================
// Global Variables
// =======================

JournalMap journalMap;
JournalAbbrMap journalAbbrMap;
InputAbbrList inputAbbrList;

static const AbbrRule abbr_rules[] = {
    {"Computer", "Comp."},
    {"Research", "Res."},
    {"Clinical", "Clin."},
    {"International", "Int."},
    {"Journal", "J."},
    {"Science", "Sci."},
    {"Technology", "Technol."},
    {"Engineering", "Eng."},
    {"Medicine", "Med."},
    {"Review", "Rev."},
    {"Studies", "Stud."},
    {"Physics", "Phys."},
    {"Chemistry", "Chem."},
    {"Biology", "Biol."},
    {"Mathematics", "Math."},
    {"Applications", "Appl."},
    {"Systems", "Syst."},
    {"Analysis", "Anal."},
    {"Education", "Educ."},
    {"Management", "Manag."},
    {"Economics", "Econ."},
    {"Multidisciplinary", "Multidiscip."},
    {"Environmental", "Environ."},
    {"Administration", "Adm."},
    {"General", "Gen."},
    {"Experimental", "Exp."},
    {"Applied", "Appl."},
    {"Developmental", "Dev."},
    {"Artificial", "AI"},
    {"Intelligence", "Intell."},
    {"Data", "Data"},
    {"Analytics", "Analyt."},
    {"Computational", "Comp."},
    {"Quantum", "Quant."},
    {"Neuroscience", "Neuro."},
    {"Genetics", "Genet."},
    {"Immunology", "Immunol."},
    {"Pharmacology", "Pharmacol."},
    {"Psychology", "Psychol."},
    {"Sociology", "Soc."},
    {"Anthropology", "Anthro."},
    {"Linguistics", "Ling."},
    {"History", "Hist."},
    {"Philosophy", "Phil."}
    // you can add more rules here
};
static const int abbr_rule_count = sizeof(abbr_rules) / sizeof(abbr_rules[0]);

// =======================
// Main Function
// =======================

int main() {
    printf("============================================\n");
    printf(" Journal Abbreviation Matcher\n");
    printf("============================================\n");
    printf("Usage:\n");
    printf("  1. Place your original journal CSV as 'orig.csv' in the current directory.\n");
    printf("  2. Place your input abbreviations as 'input.csv' in the current directory.\n");
    printf("  3. Run this program. It will generate 'output.csv' with matched full journal names.\n");
    printf("\n");
    printf("Output:\n");
    printf("  output.csv: Each row contains the original abbreviation and the best matched full journal name.\n");
    printf("\n");
    printf("Author: Haoran Zhao\n");
    printf("Contact: bill092738@gmail.com\n");
    printf("============================================\n\n");

    readCSV("orig.csv");
    normalizeName();
    generateAbbrs();
    readCSV("orig.csv"); // re-read to ensure output is based on the original data
    readINPUT("input.csv");

    printf("Processing and generating mapping... this may take a while\n");
    writeAns("input.csv", "output.csv");
    printf("Done! Please check 'output.csv' for results.\n");

    return 0;
}

// =======================
// Function Definitions
// =======================

void readCSV(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Failed to open %s for reading. Please check if the file exists and you have permission.\n", filename);
        exit(1);
    }
    char line[1024];
    // skip header
    fgets(line, sizeof(line), fp);

    // clear counts
    for (int i = 0; i < COLUMN_COUNT; ++i) journalMap.count[i] = 0;

    while (fgets(line, sizeof(line), fp)) {
        int col = 0;
        char* p = line;
        while (col < COLUMN_COUNT && *p) {
            // skip leading spaces and tabs
            while (*p == ' ' || *p == '\t') ++p;

            char field[MAX_NAME_LEN] = {0};
            int fi = 0;

            if (*p == '\"') {
                // if the field starts with a quote, read until the closing quote
                ++p;
                while (*p && !(*p == '\"' && (*(p+1) == ',' || *(p+1) == '\0' || *(p+1) == '\n' || *(p+1) == '\r'))) {
                    if (*p == '\"' && *(p+1) == '\"') { // handle escaped quotes
                        field[fi++] = '\"';
                        p += 2;
                    } else {
                        field[fi++] = *p++;
                        if (fi >= MAX_NAME_LEN - 1) break;
                    }
                }
                ++p; // jump over the closing quote
                if (*p == ',') ++p; // jump over the comma
            } else {
                // non-quoted field
                while (*p && *p != ',' && *p != '\n' && *p != '\r') {
                    field[fi++] = *p++;
                    if (fi >= MAX_NAME_LEN - 1) break;
                }
                if (*p == ',') ++p; // jump over the comma
            }

            // remove trailing spaces and tabs
            while (fi > 0 && (field[fi-1] == ' ' || field[fi-1] == '\t')) field[--fi] = '\0';

            if (fi > 0) {
                strncpy(journalMap.journals[col][journalMap.count[col]], field, MAX_NAME_LEN - 1);
                journalMap.journals[col][journalMap.count[col]][MAX_NAME_LEN - 1] = '\0';
                journalMap.count[col]++;
            }
            col++;
        }
    }
    fclose(fp);
}

void normalizeSingleName(char* name) {
    char buf[MAX_NAME_LEN];
    int j = 0;
    int in_word = 0;
    for (int i = 0; name[i] != '\0' && j < MAX_NAME_LEN - 1; ++i) {
        if (is_alnum(name[i])) {
            if (in_word == 0 && j > 0) {
                buf[j++] = ' ';
            }
            buf[j++] = name[i];
            in_word = 1;
        } else {
            in_word = 0;
        }
    }
    // remove trailing spaces
    while (j > 0 && buf[j-1] == ' ') --j;
    buf[j] = '\0';
    // copy the formatted name back
    strcpy(name, buf);
}

void normalizeName() {
    for (int col = 0; col < COLUMN_COUNT; ++col) {
        for (int i = 0; i < journalMap.count[col]; ++i) {
            normalizeSingleName(journalMap.journals[col][i]);
        }
    }
}

void generateAbbrs() {
    for (int col = 0; col < COLUMN_COUNT; ++col) {
        journalAbbrMap.count[col] = journalMap.count[col];
        for (int i = 0; i < journalMap.count[col]; ++i) {
            const char* fullname = journalMap.journals[col][i];
            journalAbbrMap.journals[col][i].fullname = fullname;

            // 1. abbreviated initials
            char words[MAX_WORDS][MAX_NAME_LEN];
            int wcount = split_words(fullname, words);
            char abbr1[MAX_NAME_LEN] = {0};
            int pos = 0;
            for (int w = 0; w < wcount; ++w) {
                abbr1[pos++] = toupper(words[w][0]);
            }
            abbr1[pos] = '\0';
            journalAbbrMap.journals[col][i].abbr[0] = strdup(abbr1);

            // 2. rules-based abbreviation
            char abbr2[MAX_NAME_LEN] = {0};
            pos = 0;
            for (int w = 0; w < wcount; ++w) {
                const char* rule = find_abbr(words[w]);
                if (rule) {
                    int len = strlen(rule);
                    if (pos > 0) abbr2[pos++] = ' ';
                    strncpy(abbr2 + pos, rule, MAX_NAME_LEN - pos - 1);
                    pos += len;
                } else {
                    if (pos > 0) abbr2[pos++] = ' ';
                    int len = strlen(words[w]);
                    strncpy(abbr2 + pos, words[w], MAX_NAME_LEN - pos - 1);
                    pos += len;
                }
            }
            abbr2[pos] = '\0';
            journalAbbrMap.journals[col][i].abbr[1] = strdup(abbr2);

            // 3. same as full name
            journalAbbrMap.journals[col][i].abbr[2] = strdup(fullname);

            // 4. first three letters of each word
            char abbr4[MAX_NAME_LEN] = {0};
            pos = 0;
            for (int w = 0; w < wcount; ++w) {
                if (pos > 0) abbr4[pos++] = ' ';
                int len = strlen(words[w]) < 3 ? strlen(words[w]) : 3;
                strncpy(abbr4 + pos, words[w], len);
                pos += len;
            }
            abbr4[pos] = '\0';
            journalAbbrMap.journals[col][i].abbr[3] = strdup(abbr4);

            // 5. only the first word of the full name
            char abbr5[MAX_NAME_LEN] = {0};
            if (wcount > 0) {
                strncpy(abbr5, words[0], MAX_NAME_LEN - 1);
                abbr5[MAX_NAME_LEN - 1] = '\0';
            }
            journalAbbrMap.journals[col][i].abbr[4] = strdup(abbr5);
        }
    }
}

void readINPUT(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Failed to open %s for reading. Please check if the file exists and you have permission.\n", filename);
        exit(1);
    }
    char line[256];
    inputAbbrList.count = 0;

    // skip header
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        // remove trailing newline characters
        char* p = line;
        while (*p && *p != '\n' && *p != '\r') ++p;
        *p = '\0';

        // clean the abbreviation
        char cleaned[MAX_INPUT_ABBR_LEN];
        cleanAbbr(line, cleaned);

        if (cleaned[0] != '\0' && inputAbbrList.count < MAX_INPUT) {
            strncpy(inputAbbrList.abbrs[inputAbbrList.count], cleaned, MAX_INPUT_ABBR_LEN - 1);
            inputAbbrList.abbrs[inputAbbrList.count][MAX_INPUT_ABBR_LEN - 1] = '\0';
            inputAbbrList.count++;
        }
    }
    fclose(fp);
}

void cleanAbbr(const char* src, char* dst) {
    int j = 0;
    for (int i = 0; src[i] != '\0' && j < MAX_INPUT_ABBR_LEN - 1; ++i) {
        if ((src[i] >= 'A' && src[i] <= 'Z') ||
            (src[i] >= 'a' && src[i] <= 'z') ||
            (src[i] == '.') ||
            (src[i] >= '0' && src[i] <= '9')) {
            dst[j++] = src[i];
        }
    }
    dst[j] = '\0';
}

int editDistance(const char* str1, const char* str2) {
    int len1 = strlen(str1);
    int len2 = strlen(str2);

    if (len1 == 0) return len2;
    if (len2 == 0) return len1;

    int** dp = (int**)malloc((len1 + 1) * sizeof(int*));
    if (dp == NULL) {
        fprintf(stderr, "Error: Memory allocation failed in editDistance.\n");
        return -1;
    }

    for (int i = 0; i <= len1; ++i) {
        dp[i] = (int*)malloc((len2 + 1) * sizeof(int));
        if (dp[i] == NULL) {
            for (int j = 0; j < i; ++j) free(dp[j]);
            free(dp);
            fprintf(stderr, "Error: Memory allocation failed in editDistance.\n");
            return -1;
        }
    }

    for (int i = 0; i <= len1; ++i) dp[i][0] = i;
    for (int j = 0; j <= len2; ++j) dp[0][j] = j;

    for (int i = 1; i <= len1; ++i) {
        for (int j = 1; j <= len2; ++j) {
            int c1 = tolower((unsigned char)str1[i - 1]);
            int c2 = tolower((unsigned char)str2[j - 1]);
            int cost = (c1 == c2) ? 0 : 1;
            int deletion = dp[i - 1][j] + 1;
            int insertion = dp[i][j - 1] + 1;
            int substitution = dp[i - 1][j - 1] + cost;

            dp[i][j] = deletion < insertion ? deletion : insertion;
            if (substitution < dp[i][j]) dp[i][j] = substitution;
        }
    }

    int result = dp[len1][len2];
    for (int i = 0; i <= len1; ++i) free(dp[i]);
    free(dp);

    return result;
}

const char* find_abbr(const char* word) {
    for (int i = 0; i < abbr_rule_count; ++i) {
        if (strcasecmp(word, abbr_rules[i].word) == 0) {
            return abbr_rules[i].abbr;
        }
    }
    return NULL;
}

int split_words(const char* name, char words[MAX_WORDS][MAX_NAME_LEN]) {
    int count = 0, i = 0, j = 0;
    while (*name && count < MAX_WORDS) {
        while (*name && !is_alnum(*name)) ++name;
        if (!*name) break;
        j = 0;
        while (*name && is_alnum(*name) && j < MAX_NAME_LEN - 1) {
            words[count][j++] = *name++;
        }
        words[count][j] = '\0';
        if (j > 0) ++count;
        while (*name && !is_alnum(*name)) ++name;
    }
    return count;
}

const char* calculateSimilarity(const char* inputAbbr) {
    if (!inputAbbr || !inputAbbr[0]) {
        fprintf(stderr, "Warning: Empty or invalid abbreviation provided to calculateSimilarity.\n");
        return NULL;
    }

    // look for the first character to determine the column
    char first = toupper((unsigned char)inputAbbr[0]);
    int col = -1;
    if (first >= 'A' && first <= 'D') col = 0;
    else if (first >= 'E' && first <= 'K') col = 1;
    else if (first >= 'L' && first <= 'O') col = 2;
    else if (first >= 'P' && first <= 'Z') col = 3;
    else {
        fprintf(stderr, "Warning: Abbreviation '%s' does not start with a valid letter (A-Z).\n", inputAbbr);
        return NULL;
    }

    int minSum = -1;
    int minIdx = -1;
    for (int i = 0; i < journalAbbrMap.count[col]; ++i) {
        int sum = 0;
        for (int k = 0; k < 5; ++k) {
            const char* abbr = journalAbbrMap.journals[col][i].abbr[k];
            int dist = editDistance(inputAbbr, abbr);

            // if the first character of the abbreviation is different from the input abbreviation
            if (abbr[0] && inputAbbr[0] &&
                toupper((unsigned char)abbr[0]) != toupper((unsigned char)inputAbbr[0])) {
                dist *= 1; // you can adjust the weight here
            }
            sum += dist;
        }
        if (minSum == -1 || sum < minSum) {
            minSum = sum;
            minIdx = i;
        }
    }
    if (minIdx >= 0)
        return journalAbbrMap.journals[col][minIdx].fullname;
    else
        return NULL;
}

void writeAns(const char* inputFile, const char* outputFile) {
    FILE* fin = fopen(inputFile, "r");
    if (!fin) {
        fprintf(stderr, "Error: Failed to open %s for reading. Please check if the file exists and you have permission.\n", inputFile);
        exit(1);
    }
    FILE* fout = fopen(outputFile, "w");
    if (!fout) {
        fclose(fin);
        fprintf(stderr, "Error: Failed to open %s for writing. Please check your permissions or disk space.\n", outputFile);
        exit(1);
    }

    char line[256];
    // read the header and write it to the output file
    if (fgets(line, sizeof(line), fin)) {
        // remove trailing newline characters
        char* p = line;
        while (*p && *p != '\n' && *p != '\r') ++p;
        *p = '\0';
        fprintf(fout, "%s,matched_fullname\n", line);
    }

    while (fgets(line, sizeof(line), fin)) {
        // remove trailing newline characters
        char* p = line;
        while (*p && *p != '\n' && *p != '\r') ++p;
        *p = '\0';

        // directly use the line as abbreviation
        char cleaned[MAX_INPUT_ABBR_LEN];
        cleanAbbr(line, cleaned);

        const char* match = NULL;
        if (cleaned[0] != '\0') {
            match = calculateSimilarity(cleaned);
        }
        fprintf(fout, "\"%s\",%s\n", line, match ? match : "");
    }

    fclose(fin);
    fclose(fout);
}

static int is_alnum(char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9');
}