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
            temperature=0.3
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
            "You are a journal name mapping machine. For the 10 line of string below, reply exactly 10 answers even if duplicated.:\n"
            "1. Convert each journal abbreviation or chaotic format name to its full name \n"
            "2. Return only the full journal name in order of 10 line in plain text.\n"
            "3. Remove any extra text, including publisher infomation or ID of journal.\n"
            "4. Be aware that some string may already be full journal names, so return them as is.\n"
            "5. Be aware that mutiple string can lead to the same journal name, so return the same journal name for them.\n"
            "6. Do not add any explain, headers, or punctuation mark or symbol of sorting. Act as a machine!\n\n"
            + "\n".join(abbreviations)
        )

        try:
            response = client.chat.completions.create(
                model="Meta-Llama-3-8B-Instruct.Q6_K",
                messages=[
                    {
                        "role": "system", 
                        "content": ("You are a journal name matching machine. "
                                  "Always provide a full journal name, even if you need to make guess based on the pattern and your knowledge."
                                  "DO NOT add any explain, headers, or punctuation mark or symbol of sorting."
                                  "For the 10 line of string below, reply exactly 10 answers even if duplicated.")
                    },
                    {"role": "user", "content": prompt}
                ],
                temperature=1  # Slightly increased to allow for more creative matching
            )
            
            # Process response content
            response_content = response.choices[0].message.content
            batch_results = [
                line.strip() for line in response_content.split('\n') 
                if line.strip()  # Remove blank lines
            ][:len(abbreviations)]  # Truncate to expected size

            # Retry logic if not enough results
            if len(batch_results) < len(abbreviations):
                print(f"Warning: Batch {i//batch_size+1} received incomplete results. Retrying this batch...")
                logger.warning(f"Batch {i//batch_size+1} received incomplete results. Retrying this batch...")
                # Retry once
                response_retry = client.chat.completions.create(
                    model="Meta-Llama-3-8B-Instruct.Q6_K",
                    messages=[
                        {
                            "role": "system", 
                            "content": ("You are a journal name matching machine. "
                                      "Always provide a full journal name, even if you need to make guess based on the pattern and your knowledge."
                                      "DO NOT add any explain, headers, or punctuation mark or symbol of sorting."
                                      "For the 10 line of string below, reply exactly 10 answers even if duplicated.")
                        },
                        {"role": "user", "content": prompt}
                    ],
                    temperature=1
                )
                response_content_retry = response_retry.choices[0].message.content
                batch_results = [
                    line.strip() for line in response_content_retry.split('\n') 
                    if line.strip()
                ][:len(abbreviations)]
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

def think_again(df, idx, client):
    """
    For a given index with 'no response', re-ask the LLM for the 9 rows above, itself, and 9 rows below (total up to 19).
    Each abbreviation is processed one by one for better accuracy.
    Returns a dict: {row_index: new_full_name}
    """
    start = max(0, idx - 9)
    end = min(len(df), idx + 10)  # +10 because end is exclusive
    indices = list(range(start, end))
    results = {}
    for i in indices:
        abbr = df.iloc[i]['abbreviation']
        new_full_name = normalize_journal_name(client, abbr)
        results[i] = new_full_name if new_full_name else "No response"
    return results

if __name__ == "__main__":
    csv_path = "input.csv"
    output_path = "journal_matches_output.csv"
    rounds = int(input("Enter number of rounds to compute (-1 for all entries): "))
    
    try:
        results = check_journal_matches(csv_path, output_path, rounds)
        print("\nCheck completed!")
        print(f"Total responses: {len(results)}")
        print(f"Results saved to: {output_path}")

        # --- Post-processing: handle "British"/"Breast" confusion ---
        df_out = pd.read_csv(output_path)
        client = OpenAI(base_url="http://127.0.0.1:8080/", api_key="none")
        updated_british_breast = False
        for i, row in df_out.iterrows():
            ans = str(row['full_name'])
            if ("British" in ans or "Breast" in ans):
                abbr = row['abbreviation']
                new_full_name = normalize_journal_name(client, abbr)
                if new_full_name and new_full_name != ans:
                    df_out.at[i, 'full_name'] = new_full_name
                    updated_british_breast = True
                    logger.info(f"Updated line {i} due to 'British'/'Breast' confusion: {ans} -> {new_full_name}")

        if updated_british_breast:
            df_out.to_csv(output_path, index=False)
            print("Updated 'British'/'Breast' confusion entries and saved to output file.")
        else:
            print("No 'British'/'Breast' confusion entries were updated.")

    except Exception as e:
        logger.error(f"An error occurred: {str(e)}")