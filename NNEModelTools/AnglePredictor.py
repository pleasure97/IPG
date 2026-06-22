import torch 
import torch.nn as nn 
import torch.optim as optim 
import pandas as pd
import numpy as np 
import os 
from sklearn.preprocessing import StandardScaler # tools for normalization 

# =====================================
# 1. Load Data and Preprocess
# =====================================

csv_name = "ProjectileTrainingData.csv"
base_dir = os.path.dirname(os.path.abspath(__file__))

# Find file path 
csv_path = os.path.join(base_dir, csv_name) if os.path.exists(csv_name) else csv_name

if not os.path.exists(csv_path):
    print(f"Error : Cannot find {csv_path}! Collect data first in unreal editor")

# Read data
data = pd.read_csv(csv_path, header=None) # If data starts from the first line
print(f"Saved total data : {len(data)} pieces")

# Drop N/A 
hit_data = data.dropna()

if (len(hit_data) < 10):
    print("Insufficient data! Please collect more success data in unreal editor.")
    exit()

# Index : [0] - distance / [1] - height difference / [2] - angle
# Input(X) : distance, height difference
# Output(Y) : angle
X = hit_data[[0, 1, 3]].values
Y = hit_data[2].values.reshape(-1, 1)

# Data standardization
scaler_X = StandardScaler()
scaler_Y = StandardScaler()

X_scaled = scaler_X.fit_transform(X) # mean = 0, std = 1 
Y_scaled = scaler_Y.fit_transform(Y) # mean = 0, std = 1

print("\n" + "=" * 50)
print("[ Normalized value ]")
print(f"Input(X) Mean : {scaler_X.mean_.tolist()}")
print(f"Input(X) Std : {scaler_X.scale_.tolist()}")
print(f"Output(Y) Mean : {scaler_Y.mean_.tolist()}")
print(f"Output(Y) Std : {scaler_Y.scale_.tolist()}")
print("=" * 50 + "\n")

X_tensor = torch.tensor(X_scaled, dtype=torch.float32)
Y_tensor = torch.tensor(Y_scaled, dtype=torch.float32)

# =====================================
# 2. Define NN Model
# =====================================
class AnglePredictor(nn.Module):
    def __init__(self):
        super(AnglePredictor, self).__init__()
        self.net = nn.Sequential(
            nn.Linear(3, 64), # the number of inputs is 2 (distance, height difference)
            nn.ReLU(), 
            nn.Linear(64, 64), 
            nn.ReLU(), 
            nn.Linear(64, 1))  # the number of output is 1 (angle)

    def forward(self, x):
        return self.net(x)

model = AnglePredictor()

# =====================================
# 3. Train Model
# =====================================

criterion = nn.MSELoss() 
optimizer = optim.Adam(model.parameters(), lr=1e-2)

print(f"Train starts...")
model.train()
for epoch in range(20000):
    prediction = model(X_tensor)
    loss = criterion(prediction, Y_tensor)

    # Double the weight for low-angle data
    weights = torch.where(X_tensor[:, 2:3] == 0, 2., 1.)
    weighted_loss = (loss * weights).mean()

    optimizer.zero_grad()
    weighted_loss.backward()
    optimizer.step()

    if (epoch + 1) % 2000 == 0:
        print(f"Epoch [{epoch + 1} / 20000], Loss : {loss.item():.6f}")

print("Train complete!")

# =====================================
# 4. Save Trained Model as onnx
# =====================================
model.eval()

with torch.no_grad():
    dummy_input = torch.randn(1, 3) # dummy data to understant the model structure
    output_filename = "AnglePredictor.onnx"
    torch.onnx.export(model, dummy_input, output_filename)

print(f"Saved the model in {os.path.abspath(output_filename)}")