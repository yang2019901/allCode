import torch
import torch.nn.functional as F
import torch.nn.init as init
import math
import numpy as np
import pandas as pd
from torch.autograd import Variable
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
#从本地读取数据
path = r'C:\Users\Lenovo\Desktop\MCM2021\MCMTest2021.csv'
# labelNumber = -2 # output3: -1
                 # output2: -2
                 # output1: -3 
xy = pd.read_csv(path).values
x = xy[:,1:-3].astype('float')
U,sigma,tV = np.linalg.svd(x)
print(U,end='\n\n')
print(sigma,end='\n\n')
print(tV)
