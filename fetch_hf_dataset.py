import os
import json
from datasets import load_dataset
from dotenv import load_dotenv

def fetch_legal_dataset():
    # Load .env variables correctly
    load_dotenv()
    hf_token = os.getenv("HF_TOKEN")
    
    if not hf_token or "YOUR_NEW_ROTATED_TOKEN" in hf_token:
        print("\033[91m[ERROR] Please insert your fresh HF_TOKEN into .env before fetching private datasets.\033[0m")
        return
        
    print("[+] Targeting: zkaedi/solidity-vulnerability-energy-signatures")
    print("[+] Downloading 2,250 sample corpus...")
    try:
        # Load the private repository
        dataset = load_dataset("zkaedi/solidity-vulnerability-energy-signatures", token=hf_token)
    except Exception as e:
        print(f"\033[91m[ERROR] Failed to fetch dataset via token!\n{e}\033[0m")
        return

    # Extract the default training split
    train_data = dataset.get('train')
    if not train_data:
        print("[ERROR] No 'train' split found in the dataset.")
        return
        
    extracted_signatures = []
    
    print(f"[*] Found {len(train_data)} legal clause energy signatures.")
    
    for row in train_data:
        extracted_signatures.append(row)

    # Saving over the expected training path so `train_evm.py` natively drops it in
    output_path = "solidity-vulnerability-energy-signatures.json"
    
    with open(output_path, 'w') as f:
        json.dump(extracted_signatures, f, indent=2)
        
    print(f"[SUCCESS] Exported 4D arrays to {output_path}.")
    print("[READY] The zkaedi_prime_layer5.py / train_evm.py loop will now automatically ingest this ground truth.")

if __name__ == "__main__":
    fetch_legal_dataset()
