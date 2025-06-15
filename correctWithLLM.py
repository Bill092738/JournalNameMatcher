import pandas as pd
from openai import OpenAI
import time

def check_journal_matches(csv_file_path, rounds=-1, batch_size=10):
    """
    Check journal name matches using local LLM API
    Args:
        csv_file_path (str): Path to CSV file
        rounds (int): Number of rounds to check (-1 for all entries)
        batch_size (int): Number of entries to check per API call
    """
    # Read CSV file
    df = pd.read_csv(csv_file_path)
    
    # Initialize OpenAI client
    client = OpenAI(base_url="http://127.0.0.1:8080/", api_key="none")
    
    # Calculate total entries to process
    total_entries = len(df) if rounds == -1 else min(rounds * batch_size, len(df))
    
    results = []
    
    for i in range(0, total_entries, batch_size):
        batch = df.iloc[i:i + batch_size]
        
        # Construct prompt for batch
        prompt = "For each of the following journal name pairs, indicate if the matching is correct. If incorrect, provide the correct full name:\n\n"
        for _, row in batch.iterrows():
            prompt += f"Abbreviated: {row['abbreviation']}\n"
            prompt += f"Matched: {row['matched_fullname']}\n"
            prompt += "---\n"
        
        try:
            response = client.chat.completions.create(
                model="gpt-3.5-turbo",
                messages=[
                    {"role": "system", "content": "You are a journal name matching expert. Evaluate each pair and respond with either 'CORRECT' or provide the correct full name if the matching is wrong."},
                    {"role": "user", "content": prompt}
                ],
                temperature=0.3
            )
            
            print(f"\nProcessing batch {i//batch_size + 1}:")
            print(response.choices[0].message.content)
            results.extend(response.choices[0].message.content.split('\n'))
            
            # Add a small delay to avoid overwhelming the API
            time.sleep(1)
            
        except Exception as e:
            print(f"Error processing batch starting at index {i}: {str(e)}")
            continue
    
    return results

if __name__ == "__main__":
    # Example usage
    csv_path = "output.csv"
    
    # Ask user for number of rounds
    rounds_input = input("Enter number of rounds to check (or -1 for all entries): ")
    rounds = int(rounds_input)
    
    results = check_journal_matches(csv_path, rounds)
    
    print("\nCheck completed!")
    print(f"Total responses processed: {len(results)}")