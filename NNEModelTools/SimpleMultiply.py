import torch
import torch.nn as nn
import torch.onnx
import os 

# 1. Define simple NN model
# Inherit nn.Module
class SimpleMultiply2Model(nn.Module):
    def __init__(self): 
        super(SimpleMultiply2Model, self).__init__()
        self.linear = nn.Linear(1, 1, bias=False)
        self.linear.weight.data.fill_(2.)

    def forward(self, x):
        return self.linear(x)

# 2. Create model instance 
model = SimpleMultiply2Model()
# 3. Switch to evaluation mode
model.eval()

# 4. Tracing - Dummy data to understand the model structure
dummy_input = torch.randn(1, 1)

# 5. Export to .onnx file 
output_filename = "SimpleMultiply2Model.onnx"
torch.onnx.export(model, dummy_input, output_filename)

# 6. Print log
print(f"Success : Created in {os.path.abspath(output_filename)}")
