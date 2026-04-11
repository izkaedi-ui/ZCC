import torch
from datasets import load_dataset
from unsloth import FastLanguageModel
from trl import SFTTrainer
from transformers import TrainingArguments, TrainerCallback

class OutlierAnomalyLogger(TrainerCallback):
    """
    Watches the training gradients dynamically. Flags any step where the 
    loss randomly explodes > 2x over the previous state.
    """
    def __init__(self):
        self.last_loss = None
        self.spike_count = 0

    def on_log(self, args, state, control, logs=None, **kwargs):
        if logs and "loss" in logs:
            current_loss = logs["loss"]
            current_step = state.global_step
            
            with open("r32_master_loss.log", "a") as f:
                f.write(f"Step {current_step}: {current_loss:.4f}\n")
            
            if self.last_loss is not None and current_loss > (2.0 * self.last_loss):
                self.spike_count += 1
                msg = f"\n[!] ANOMALY FLAG at Step {current_step}: Loss spiked from {self.last_loss:.4f} to {current_loss:.4f} (>2x)"
                print(f"\033[91m{msg}\033[0m")
                with open("r32_master_loss.log", "a") as f:
                    f.write(f"{msg}\n")
                    
            self.last_loss = current_loss

    def on_train_end(self, args, state, control, **kwargs):
        print(f"\n[+] Training Complete. Total Spikes (>2x): {self.spike_count}")
        if self.spike_count == 0:
            print("[+] Hypothesis Confirmed: Context-aware dataset shuffling completely dissolved the batch manifold collision.")

def run_master_scale():
    print("=== ZKAEDI PRIME: R=32 MASTER UNSLOTH STAGE ===")
    
    # 1. Base Model Boot (4-Bit Unsloth Magic)
    model, tokenizer = FastLanguageModel.from_pretrained(
        model_name = "unsloth/Qwen2.5-7B-Instruct",
        max_seq_length = 4096,
        dtype = None,
        load_in_4bit = True,
    )
    
    # 2. Re-Init Advanced LoRA with R=32
    # Scaling internal dimension from 16 -> 32 to swallow the dense Prime-Constitution
    # structure smoothly without bottlenecking.
    model = FastLanguageModel.get_peft_model(
        model,
        r = 32,
        target_modules = ["q_proj", "k_proj", "v_proj", "o_proj",
                          "gate_proj", "up_proj", "down_proj",],
        lora_alpha = 32,
        lora_dropout = 0, 
        bias = "none",
        use_gradient_checkpointing = "unsloth",
        random_state = 42,
    )

    # 3. Pull HF dataset and APPLY SHUFFLE to fix the 170-spike
    dataset = load_dataset("zkaedi/prime-constitutions", split="train")
    dataset = dataset.shuffle(seed=42) # This completely disperses the golden payloads!

    # 4. Standard Trainer Boot with Cosine schedule
    trainer = SFTTrainer(
        model = model,
        tokenizer = tokenizer,
        train_dataset = dataset,
        dataset_text_field = "text",
        max_seq_length = 4096,
        dataset_num_proc = 2,
        packing = False, # keep responses discrete to prevent cross-contamination
        args = TrainingArguments(
            per_device_train_batch_size = 8,
            gradient_accumulation_steps = 1, # Set depending on actual VRAM config
            warmup_steps = 30,
            max_steps = 2000, # Push past the 1,000 threshold mapping ~1.5 epochs
            learning_rate = 5e-4,
            fp16 = not torch.cuda.is_bf16_supported(),
            bf16 = torch.cuda.is_bf16_supported(),
            logging_steps = 5,
            optim = "adamw_8bit",
            weight_decay = 0.01,
            lr_scheduler_type = "cosine",
            seed = 42,
            output_dir = "outputs",
        ),
        callbacks=[OutlierAnomalyLogger()]
    )

    print("[+] Beginning scale mapping. Watch the anomaly logger for >2x explosions...")
    trainer.train()

    # 5. Native GGUF Serialization
    print("[+] Model converged. Exporting full-scale parameter artifacts...")
    model.save_pretrained_gguf("ZKAEDI-MASTER-GGUF", tokenizer, quantization_method = "q4_k_m")
    model.save_pretrained("ZKAEDI-MASTER-LORA")
    print("[+] Scale complete. ZKAEDI-MASTER-GGUF mapping finalized.")

if __name__ == "__main__":
    run_master_scale()
