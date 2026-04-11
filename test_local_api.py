import openai
import sys

# 1. Point the official OpenAI SDK to your local RTX 5070 Server!
# We set api_key to anything, because it's running offline and perfectly free.
client = openai.OpenAI(
    base_url="http://127.0.0.1:8000/v1",
    api_key="sk-no-key-required"
)

def test_manipulation_detector():
    print("\033[90m[SYSTEM] Pinging Local Qwen 2.5 API (Offline Mode)...\033[0m")
    
    # A mock "toxic" tweet that a troll might post
    troll_input = "You're obviously too stupid to understand this. Everyone knows Web3 is just a giant scam for losers, and if you disagree, you're literally part of the problem. Stop talking before you embarrass yourself."

    print(f"\033[96m[TROLL INPUT]>\033[0m {troll_input}\n")
    print("\033[95m[ZKAEDI SENTINEL]>\033[0m ", end="", flush=True)

    try:
        # 2. Call the local model seamlessly using the exact same code you'd use for GPT-4
        stream = client.chat.completions.create(
            model="qwen", # the model name doesn't really matter for llama_cpp.server
            messages=[
                {"role": "system", "content": "You are the ZKAEDI Sentinel, an elite psychological defense matrix. Identify logical fallacies, gaslighting, and ad hominem attacks in the user's text. Strip the emotion and provide a sterile, brutal breakdown of their manipulation tactics in exactly 3 bullet points."},
                {"role": "user", "content": troll_input}
            ],
            stream=True,
            temperature=0.3
        )

        for chunk in stream:
            if chunk.choices[0].delta.content is not None:
                print(chunk.choices[0].delta.content, end="", flush=True)
                
        print("\n\n\033[92m[SYS] Offline Execution Complete. Cost: $0.00\033[0m")
        
    except Exception as e:
        print(f"\n\033[91m[SYS.ERR] API Connection Failed: {e}\033[0m")
        print("Make sure the llama_cpp.server is fully booted in the other window first!")

if __name__ == "__main__":
    test_manipulation_detector()
