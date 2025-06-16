#!/bin/bash

set -e

echo "=== JournalNameMatcher One-Click Runner ==="

# 1. Install dependencies
echo "[1/5] Installing Python dependencies..."
pip install --user pandas openai

# 2. Compile normalizer
echo "[2/5] Compiling normalizer.cpp..."
g++ -O3 -march=native -funroll-loops -flto -s -pipe -o normalizer normalizer.cpp

# 3. Run normalizer to preprocess input
echo "[3/5] Running normalizer..."
./normalizer

# 4. Run LLM mapping (requires local LLM API or OpenAI API)
echo "[4/5] Running MapWithLLM.py (make sure your LLM API is running)..."
python3 MapWithLLM.py

# 5. Expand results
echo "[5/5] Running reorganizer.py to generate expanded output..."
python3 reorganizer.py

echo "=== All done! Check journal_matches_expanded.csv for results. ==="