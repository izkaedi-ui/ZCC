import os
import glob
import json
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader, TensorDataset

class SurrogateAttractorModel(nn.Module):
    def __init__(self):
        super(SurrogateAttractorModel, self).__init__()
        self.net = nn.Sequential(
            nn.Linear(10, 64),
            nn.ReLU(),
            nn.Linear(64, 128),
            nn.ReLU(),
            nn.Linear(128, 64),
            nn.ReLU(),
            nn.Linear(64, 2)
        )
    def forward(self, x):
        return self.net(x)

def load_data(directory):
    X, y = [], []
    files = glob.glob(os.path.join(directory, "zkaedi_attractors_*.jsonl"))
    for f_path in files:
        with open(f_path, 'r') as f:
            for line in f:
                if not line.strip(): continue
                data = json.loads(line)
                st = data.get("state", {})
                vec = [st.get("eta",0.0), st.get("gamma",0.0), st.get("beta",0.0), st.get("sigma",0.0),
                       st.get("kappa",0.0), st.get("alpha",0.0), st.get("omega",0.0), st.get("tau",0.0),
                       st.get("nu",0.0), st.get("mu",0.0)]
                X.append(vec)
                y.append([data.get("fitness",0.0), data.get("tensorPeak",0.0)])
    return torch.tensor(X, dtype=torch.float32), torch.tensor(y, dtype=torch.float32)

if __name__ == "__main__":
    import time
    directory = r"d:\discovered_algorithms_extracted\discovered_algorithms"
    X, y = load_data(directory)
    if len(X) == 0: exit(1)

    print(f"Loaded {len(X)} samples. Normalizing...")
    X_mean = X.mean(dim=0)
    X_std = X.std(dim=0)
    X_std[X_std == 0] = 1.0
    y_mean = y.mean(dim=0)
    y_std = y.std(dim=0)
    y_std[y_std == 0] = 1.0
    
    X_norm = (X - X_mean) / X_std
    y_norm = (y - y_mean) / y_std

    models_dir = os.path.join(directory, "models")
    os.makedirs(models_dir, exist_ok=True)
    with open(os.path.join(models_dir, "scaler.json"), "w") as f:
        json.dump({"X_mean": X_mean.tolist(), "X_std": X_std.tolist(), "y_mean": y_mean.tolist(), "y_std": y_std.tolist()}, f)
    
    dataset = TensorDataset(X_norm, y_norm)
    loader = DataLoader(dataset, batch_size=32, shuffle=True)
    
    model = SurrogateAttractorModel()
    criterion = nn.MSELoss()
    optimizer = optim.Adam(model.parameters(), lr=0.005)
    
    start_t = time.time()
    for epoch in range(400):
        epoch_loss = 0.0
        for bx, by in loader:
            optimizer.zero_grad()
            loss = criterion(model(bx), by)
            loss.backward()
            optimizer.step()
            epoch_loss += loss.item()
            
    print(f"Norm Training completed in {time.time() - start_t:.2f}s. Final Norm Loss: {epoch_loss/len(loader):.4f}")
    torch.save(model.state_dict(), os.path.join(models_dir, "surrogate_attractor_model_norm.pth"))
    
    # Simple explicit verify step within the script itself:
    model.eval()
    with torch.no_grad():
        test_out = model(X_norm[0].unsqueeze(0))
        pred_y = test_out * y_std + y_mean
        print(f"\n[Normalized Inference Test on Sample 0]")
        print(f"Target : {y[0].tolist()}")
        print(f"Predict: {pred_y[0].tolist()}")
