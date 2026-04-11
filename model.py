import torch
import torch.nn as nn

class CompactCNN(nn.Module):
    """
    ZKAEDI PRIME CompactCNN.
    A VGG-style 3-block convolutional neural network natively optimized for rapid iteration.
    Natively supports CIFAR-10 inputs (3, 32, 32) tracking to 10 class outputs.
    Total Parameters: ~3.25 Million.
    """
    def __init__(self, num_classes=10):
        super(CompactCNN, self).__init__()
        
        # We define features as a Sequential containing Sequential sub-blocks
        # to exactly match the state_dict signature: `features.X.Y.weight`
        self.features = nn.Sequential(
            # Block 0
            nn.Sequential(
                nn.Conv2d(3, 64, kernel_size=3, padding=1),     # 0
                nn.BatchNorm2d(64),                             # 1
                nn.ReLU(inplace=True),                          # 2
                nn.Conv2d(64, 64, kernel_size=3, padding=1),    # 3
                nn.BatchNorm2d(64),                             # 4
                nn.ReLU(inplace=True),                          # 5
                nn.MaxPool2d(kernel_size=2, stride=2)           # 6 -> output (64, 16, 16)
            ),
            # Block 1
            nn.Sequential(
                nn.Conv2d(64, 128, kernel_size=3, padding=1),   # 0
                nn.BatchNorm2d(128),                            # 1
                nn.ReLU(inplace=True),                          # 2
                nn.Conv2d(128, 128, kernel_size=3, padding=1),  # 3
                nn.BatchNorm2d(128),                            # 4
                nn.ReLU(inplace=True),                          # 5
                nn.MaxPool2d(kernel_size=2, stride=2)           # 6 -> output (128, 8, 8)
            ),
            # Block 2
            nn.Sequential(
                nn.Conv2d(128, 256, kernel_size=3, padding=1),  # 0
                nn.BatchNorm2d(256),                            # 1
                nn.ReLU(inplace=True),                          # 2
                nn.Conv2d(256, 256, kernel_size=3, padding=1),  # 3
                nn.BatchNorm2d(256),                            # 4
                nn.ReLU(inplace=True),                          # 5
                nn.MaxPool2d(kernel_size=2, stride=2)           # 6 -> output (256, 4, 4)
            )
        )
        
        self.classifier = nn.Sequential(
            nn.Linear(256 * 4 * 4, 512),                        # 0 (4096 -> 512)
            nn.ReLU(inplace=True),                              # 1
            nn.Dropout(p=0.5),                                  # 2
            nn.Linear(512, num_classes)                         # 3 (512 -> 10)
        )

    def forward(self, x):
        x = self.features(x)
        x = torch.flatten(x, 1)
        x = self.classifier(x)
        return x

if __name__ == "__main__":
    # Test tensor alignment mathematically
    model = CompactCNN()
    dummy_input = torch.randn(1, 3, 32, 32)
    output = model(dummy_input)
    print("Testing Architecture Mapping...")
    print(f"  Input Shape:  {dummy_input.shape}")
    print(f"  Output Shape: {output.shape}")
    print(f"  Total Params: {sum(p.numel() for p in model.parameters()):,}")
    
    # Assert perfectly structured to cleanly absorb ULTIMA state_dict mappings
    try:
        ckpt = torch.load("ZKAEDI_ULTIMA_FUSED.pt", map_location="cpu", weights_only=True)
        sd = ckpt.get("model", ckpt) if isinstance(ckpt, dict) else ckpt
        model.load_state_dict(sd)
        print("  [✓] Successfully ingested ULTIMA_FUSED checkpoint! Matrix aligns flawlessly.")
    except Exception as e:
        print(f"  [!] Mapping structural failure: {e}")
