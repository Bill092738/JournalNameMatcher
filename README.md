# JournalNameMatcher
Map your Journal Abbreviations to Full Name

A program to map journal abbreviations to their full journal names using fuzzy matching and abbreviation rules.

## Usage

### 1. Clone the Repository

**Linux / MacOS / Windows (with Git Bash):**
```sh
git clone https://github.com/Bill092738/JournalNameMatcher.git
cd JournalNameMatcher
```

---

### 2. Install GCC/G++ Compiler

#### **Linux (Debian/Ubuntu):**
```sh
sudo apt update
sudo apt install build-essential
```

#### **MacOS (with Homebrew):**
```sh
brew install gcc
```

#### **Windows (with MinGW):**
- Download and install [MinGW](https://www.mingw-w64.org/).
- Add `C:\MinGW\bin` to your system PATH.
- Open the MinGW terminal or Command Prompt.

---

### 3. Prepare Input Files

- Place your original journal list as `orig.csv` and your abbreviations as `input.csv` in the project directory.

---

### 4. Compile the Program

#### **Linux / MacOS:**
```sh
gcc -o nameMap nameMap.c
```

#### **Windows (MinGW):**
```sh
gcc -o nameMap.exe nameMap.c
```

---

### 5. Run the Program

#### **Linux / MacOS:**
```sh
./nameMap
```

#### **Windows (MinGW):**
```sh
nameMap.exe
```

- The results will be written to `output.csv`, mapping each abbreviation to the best-matched full journal name.

---

### 6. (Optional) Calculate Similarity Scores

#### **Linux / MacOS:**
```sh
g++ -o similarityCalculator similarityCalculator.cpp
./similarityCalculator
```

#### **Windows (MinGW):**
```sh
g++ -o similarityCalculator.exe similarityCalculator.cpp
similarityCalculator.exe
```

- This will generate `output_with_similarity.csv` with similarity scores for each mapping.

## Files

- `nameMap.c` — Main source code.
- `orig.csv` — CSV with full journal names.
- `input.csv` — CSV with journal abbreviations.
- `output.csv` — Output with matched results.
