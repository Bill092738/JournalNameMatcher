import csv
import re

def clean_full_name(full):
    # Remove leading order numbers (e.g., "1. ", "2. ", "10. "), bullets, or dashes
    return re.sub(r'^\s*(?:\d+\.\s*|[-•*]\s*)+', '', full).strip()

# Read counts from temp.csv
counts = {}
with open('temp.csv', newline='', encoding='utf-8') as tempfile:
    reader = csv.DictReader(tempfile)
    for row in reader:
        abbr = row['abbreviation']
        try:
            count = int(row['count'])
            counts[abbr] = count
        except (ValueError, KeyError):
            continue

# Read mappings from journal_matches_output.csv
mappings = []
with open('journal_matches_output.csv', newline='', encoding='utf-8') as mapfile:
    reader = csv.DictReader(mapfile)
    for row in reader:
        abbr = row['abbreviation']
        full = row['full_name']
        mappings.append((abbr, full))

# Write expanded output
with open('journal_matches_expanded.csv', 'w', newline='', encoding='utf-8') as outfile:
    writer = csv.writer(outfile)
    writer.writerow(['abbreviation', 'full_name'])
    for abbr, full in mappings:
        repeat = counts.get(abbr, 1)
        cleaned_full = clean_full_name(full)
        for _ in range(repeat):
            writer.writerow([abbr, cleaned_full])