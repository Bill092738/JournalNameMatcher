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
    # Read CSV file and validate required column
    df = pd.read_csv(csv_file_path)
    if 'abbreviation' not in df.columns:
        raise KeyError("CSV must contain 'abbreviation' column")

    # Initialize OpenAI client
    client = OpenAI(base_url="http://127.0.0.1:8080/", api_key="none")
    
    # Calculate total entries to process
    total_entries = len(df) if rounds == -1 else min(rounds * batch_size, len(df))
    
    results = []
    
    for i in range(0, total_entries, batch_size):
        batch = df.iloc[i:i + batch_size]
        abbreviations = batch['abbreviation'].tolist()
        
        # Build structured prompt (one abbreviation per line)
        prompt = (
            "Provide ONLY the full name for each journal abbreviation below. "
            "Return exactly one full name per line, in the same order. "
            "DO NOT add any other text or numbering.\n\n"
            + "\n".join(abbreviations)
        )
        
        try:
            response = client.chat.completions.create(
                model="Meta-Llama-3-8B-Instruct.Q6_K",
                messages=[
                    {
                        "role": "system", 
                        "content": "You are a journal name matching expert. Return only full names."
                    },
                    {"role": "user", "content": prompt}
                ],
                temperature=0.3
            )
            
            # Process response content
            response_content = response.choices[0].message.content
            batch_results = [
                line.strip() for line in response_content.split('\n') 
                if line.strip()  # Remove blank lines
            ][:len(abbreviations)]  # Truncate to expected size
            
            results.extend(batch_results)
            print(f"Batch {i//batch_size+1}: Processed {len(batch_results)} entries")
            
            time.sleep(0.3)  # Avoid API rate limits
            
        except Exception as e:
            print(f"Error processing batch starting at index {i}: {str(e)}")
            # Add placeholders for failed batch
            results.extend([f"Error: {str(e)}"] * min(batch_size, total_entries-i))
            continue
    
    return results

if __name__ == "__main__":
    csv_path = "input.csv"
    rounds = int(input("Enter number of rounds to compute (-1 for all entries): "))
    
    results = check_journal_matches(csv_path, rounds)
    print("\nCheck completed!")
    print(f"Total responses: {len(results)}")