import os, glob, json, torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader, TensorDataset

class SurrogateAttractorModel(nn.Module):
    def __init__(self):
        super(SurrogateAttractorModel, self).__init__()
        self.net = nn.Sequential(nn.Linear(10, 64), nn.ReLU(), nn.Linear(64, 128), nn.ReLU(), nn.Linear(128, 64), nn.ReLU(), nn.Linear(64, 2))
    def forward(self, x): return self.net(x)

directory = r"d:\discovered_algorithms_extracted\discovered_algorithms"
X, y = [], []
for f_path in glob.glob(os.path.join(directory, "zkaedi_attractors_*.jsonl")):
    with open(f_path, 'r') as f:
        for line in f:
            if not line.strip(): continue
            d = json.loads(line)
            st = d.get("state", {})
            X.append([st.get("eta",0.0), st.get("gamma",0.0), st.get("beta",0.0), st.get("sigma",0.0), st.get("kappa",0.0), st.get("alpha",0.0), st.get("omega",0.0), st.get("tau",0.0), st.get("nu",0.0), st.get("mu",0.0)])
            y.append([d.get("fitness",0.0), d.get("tensorPeak",0.0)])

X, y = torch.tensor(X, dtype=torch.float32), torch.tensor(y, dtype=torch.float32)
X_m, X_s = X.mean(dim=0), X.std(dim=0)
X_s[X_s == 0] = 1.0
y_m, y_s = y.mean(dim=0), y.std(dim=0)
y_s[y_s == 0] = 1.0
X_n, y_n = (X - X_m) / X_s, (y - y_m) / y_s

md = os.path.join(directory, "models")
with open(os.path.join(md, "scaler.json"), "w") as f:
    json.dump({"X_mean": X_m.tolist(), "X_std": X_s.tolist(), "y_mean": y_m.tolist(), "y_std": y_s.tolist()}, f)

model = SurrogateAttractorModel()
optimizer = optim.Adam(model.parameters(), lr=0.005)
loader = DataLoader(TensorDataset(X_n, y_n), batch_size=32, shuffle=True)

for epoch in range(400):
    for bx, by in loader:
        optimizer.zero_grad()
        loss = nn.MSELoss()(model(bx), by)
        loss.backward()
        optimizer.step()

torch.save(model.state_dict(), os.path.join(md, "surrogate_attractor_model_norm.pth"))

model.eval()
with torch.no_grad():
    pred = model(X_n[0].unsqueeze(0)) * y_s + y_m
    print("----- NORMALIZED INFERENCE -----")
    print("Actual:  ", [round(v, 2) for v in y[0].tolist()])
    print("Predict: ", [round(v, 2) for v in pred[0].tolist()])
