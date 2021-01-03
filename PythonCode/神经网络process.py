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
labelNumber = -3 # output3: -1
                 # output2: -2
                 # output1: -3 
xy = pd.read_csv(path).values
x = xy[:,[1,3,5,6,7,8]].astype('float')
y = xy[:, labelNumber].astype('float')
#提取D1.1 D2.1 D3.1的内容
x = Variable(torch.from_numpy(x))
# 提取output1,2,3的内容
y = Variable(torch.from_numpy(y))        

#划分训练数据和测试数据
x_train, x_test,y_train,y_test= train_test_split(x.numpy(),y.numpy(), test_size=0.3,random_state=2018)   
ss = StandardScaler()  

#特征归一化
x_train = Variable(torch.tensor(ss.fit_transform(x_train)))
x_test = Variable(torch.tensor(ss.fit_transform(x_test)))
y_train = Variable(torch.tensor(y_train))
y_test = Variable(torch.tensor(y_test))

#搭建并训练神经网络
class Model(torch.nn.Module):
    def __init__(self):
        super(Model, self).__init__()
        self.l1 = torch.nn.Linear(6, 3)  
        self.l2 = torch.nn.Linear(3, 10)
        self.l3 = torch.nn.Linear(10, 1)
    def forward(self, x):
        x = F.tanh(self.l1(x.float()))
        x = F.tanh(self.l2(x))
        x = self.l3(x)
        return x
model = Model()
# model.apply(weights_init)
criterion = torch.nn.BCEWithLogitsLoss()
optimizer = torch.optim.SGD(model.parameters(), lr=1e-1, momentum=0.9)
Loss = []
Acc = []
EPOCHS = 5000
for epoch in range(EPOCHS):
    y_pred = model(x_train)
    loss = criterion(y_pred, y_train.float().view(-1,1))
    preds = torch.tensor(y_pred >= 0)
    corrects = torch.sum(preds.byte() == y_train.view(-1,1).byte())
    acc = corrects.item()/len(x_train)
    if epoch%200 == 0:
        print("corrects:",corrects)
        print("epoch = {0}, loss = {1}, acc = {2}".format(epoch, loss, acc))
        Loss.append(loss)
        Acc.append(acc)
    optimizer.zero_grad()
    loss.backward()
    optimizer.step()
plt.plot(range(len(Loss)), Loss)
plt.ylabel('loss')
plt.xlabel('epochs')
plt.show()
plt.plot(range(len(Acc)), Acc)
plt.ylabel('acc')
plt.xlabel('epochs')
plt.show()

y_pred = model(x_test)
preds = torch.tensor(y_pred >= 0)
corrects = torch.sum(preds.byte() == y_test.view(-1,1).byte())
acc = corrects.item()/len(x_test)
print("corrects:",corrects.numpy().item())
print("acc = {}".format(acc))
