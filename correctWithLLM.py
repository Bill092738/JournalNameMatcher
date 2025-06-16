import pandas as pd
from openai import OpenAI
import logging
import time

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

def normalize_journal_name(client, abbreviation):
    """Process a single journal abbreviation using the LLM."""
    prompt = (
        f"Convert this journal abbreviation or chaotic format name to its full name: {abbreviation}\n"
        "Return only the full journal name, nothing else. Act as a machine."
    )
    
    try:
        response = client.chat.completions.create(
            model="Meta-Llama-3-8B-Instruct.Q6_K",
            messages=[
                {"role": "system", "content": "You are a journal name mapping machine. Provide full journal names. Act as a machine."},
                {"role": "user", "content": prompt}
            ],
            temperature=0.1
        )
        return response.choices[0].message.content.strip()
    except Exception as e:
        logger.error(f"Error processing {abbreviation}: {str(e)}")
        return None

def check_journal_matches(csv_file_path, output_file_path, rounds=-1, batch_size=10):
    """
    Check journal name matches using local LLM API
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
    processed_abbreviations = []
    
    for i in range(0, total_entries, batch_size):
        # Calculate actual batch size for this iteration
        current_batch_size = min(batch_size, total_entries - i)
        batch = df.iloc[i:i + current_batch_size]
        abbreviations = batch['abbreviation'].tolist()
        
        # Build structured prompt (one abbreviation per line)
        prompt = (
            "You are a journal name mapping machine. For the 10 line of string below:\n"
            "1. Convert each journal abbreviation or chaotic format name to its full name \n"
            "2. Return only the full journal name in order of 10 line in plain text.\n"
            "3. Remove any extra text, including publisher infomation of journal.\n"
            "4. Do not add any explain, headers, or punctuation mark or symbol of sorting. Act as a machine.\n\n"
            + "\n".join(abbreviations)
        )

        try:
            response = client.chat.completions.create(
                model="Meta-Llama-3-8B-Instruct.Q6_K",
                messages=[
                    {
                        "role": "system", 
                        "content": ("You are a journal name matching expert. "
                                  "Always provide a full journal name, even if you need to make "
                                  "an educated guess based on the abbreviation pattern and your "
                                  "knowledge of academic journals.")
                    },
                    {"role": "user", "content": prompt}
                ],
                temperature=0.3  # Slightly increased to allow for more creative matching
            )
            
            # Process response content
            response_content = response.choices[0].message.content
            batch_results = [
                line.strip() for line in response_content.split('\n') 
                if line.strip()  # Remove blank lines
            ][:len(abbreviations)]  # Truncate to expected size
            
            # Ensure batch results match the current batch size
            if len(batch_results) < len(abbreviations):
                batch_results.extend(['No response'] * (len(abbreviations) - len(batch_results)))
            
            results.extend(batch_results)
            processed_abbreviations.extend(abbreviations)
            logger.info(f"Batch {i//batch_size+1}: Processed {len(batch_results)} entries")
            
            time.sleep(0.3)  # Avoid API rate limits
            
        except Exception as e:
            logger.error(f"Error processing batch starting at index {i}: {str(e)}")
            results.extend(['Error: ' + str(e)] * len(abbreviations))
            processed_abbreviations.extend(abbreviations)
            continue
    
    # Create output DataFrame with processed data only
    output_df = pd.DataFrame({
        'abbreviation': processed_abbreviations[:total_entries],
        'full_name': results[:total_entries]
    })
    
    # Save to CSV file
    output_df.to_csv(output_file_path, index=False)
    logger.info(f"Results saved to {output_file_path}")
    return results[:total_entries]

if __name__ == "__main__":
    csv_path = "input.csv"
    output_path = "journal_matches_output.csv"
    rounds = int(input("Enter number of rounds to compute (-1 for all entries): "))
    
    try:
        results = check_journal_matches(csv_path, output_path, rounds)
        print("\nCheck completed!")
        print(f"Total responses: {len(results)}")
        print(f"Results saved to: {output_path}")
    except Exception as e:
        logger.error(f"An error occurred: {str(e)}")