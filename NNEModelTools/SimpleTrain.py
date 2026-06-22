import torch
import torch.nn as nn 
import torch.optim as optim 
import os 

# 1. Prepare data (input x and label y)
# Learn to multiply 2
x_train = torch.tensor([[1.], [2.], [3.], [4.]])
y_train = torch.tensor([[2.], [4.], [6.], [8.]])

# 2. Define a model 
model = nn.Linear(1, 1, bias=False) # y = wx + b

# 3. Setup loss function and optimizer
criterion = nn.MSELoss()
optimizer = optim.SGD(model.parameters(), lr=1e-2)

# 4. Train the model 
model.train()
for epoch in range(1000):
    prediction = model(x_train)
    loss = criterion(prediction, y_train)
    optimizer.zero_grad()
    loss.backward()
    optimizer.step()

print(f"Train complete! Find weights : {model.weight.item():.4f}")

# 5. Switch to evlauation model
model.eval()

# 6. Export trained model to file (.onnx)
with torch.no_grad():
    dummy_input = torch.randn(1, 1) # input dummy data to understand the model structure
    output_filename = "SimpleTrain.onnx"
    torch.onnx.export(model, dummy_input, output_filename)

print(f"Saved file in {os.path.abspath(output_filename)}")