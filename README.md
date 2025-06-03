# JournalNameMatcher
Map your Journal Abbreviations to Full Name

A program to map journal abbreviations to their full journal names using fuzzy matching and abbreviation rules.

## Usage

1. Place your original journal list as `orig.csv` and your abbreviations as `input.csv` in the project directory.
2. Compile the program:

**Linux / MacOS:**
In Terminal, directly compile and run with `gcc` or `clang`.
```sh
gcc -o nameMap nameMap.c
./nameMap
```

**Windows (MinGW):**
Please make sure you have the correct MinGW environment before compiling the source code and run in Powershell.
```sh
gcc -o nameMap.exe nameMap.c
nameMap.exe
```

3. The results will be written to `output.csv`, mapping each abbreviation to the best-matched full journal name.

4. *(Optional)* To calculate similarity scores for the matched results, compile and run the similarity calculator:
```sh
g++ -o similarityCalculator similarityCalculator.cpp
./similarityCalculator
```
This will generate `output_with_similarity.csv` with similarity scores for each mapping.

## Files

- `nameMap.c` — Main source code.
- `orig.csv` — CSV with full journal names.
- `input.csv` — CSV with journal abbreviations.
- `output.csv` — Output with matched results.
