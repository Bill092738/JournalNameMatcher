from openai import OpenAI

# Test API connection to http://127.0.0.1:8080/ with key: "none"
def test_api_connection():
    client = OpenAI(base_url="http://127.0.0.1:8080/", api_key="none")
    try:
        response = client.chat.completions.create(
            model="gpt-3.5-turbo",
            messages=[{"role": "system", "content": "You are a helpful assistant."},
                      {"role": "user", "content": "Hello, how are you?"}]
        )
        print("API connection successful:", response.choices[0].message.content)
    except Exception as e:
        print("API connection failed:", str(e))
if __name__ == "__main__":
    test_api_connection()
    # The rest of your code can go here, such as calling the LLM for journal name correction
    # For example:
    # correct_journal_name("Some Journal Name")