#! /usr/bin/env python3
def warn(*args, **kwargs):
    pass
import warnings
#comment out line below if you want warnings from scikitlearn 
warnings.warn = warn

#imports
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import pylab
from mpl_toolkits.mplot3d import Axes3D
import math
import collections
import random
from sklearn.model_selection import RepeatedKFold
from sklearn.neural_network import MLPClassifier
from sklearn.decomposition import PCA

#standard is 'relu'
activationGlobal=['relu', 'tanh', 'logistic', 'identity']
#standard is 'adam'
solverGlobal=['lbfgs', 'sgd', 'adam']
#standard is .00001
alphaGlobal=[0.0001, .00001, 0.000001]
#standard is 200
max_iterationsGlobal=[100, 200, 300]

TRIALS = 30
f = open('output3.txt', 'w')


def readData():
    X = []
    y = []
    allData = np.loadtxt(fname='./437data.txt', delimiter=',', dtype=float)
    for lines in allData:
        X.append(lines[:-1])
        y.append(lines[-1])
    return X, y


def mlpKFold(X, y, k, act, solve, alph, it):
    acc = 0
    rkf = RepeatedKFold(n_splits=k)
    for train_index, test_index in rkf.split(X):
        X_train = []
        X_test = []
        y_train = []
        y_test = []
        for i in train_index:
            X_train.append(X[i])
            y_train.append(y[i])
        for i in test_index:
            X_test.append(X[i])
            y_test.append(y[i])
        mlp = MLPClassifier(activation=activationGlobal[act], solver=solverGlobal[solve], alpha=alphaGlobal[alph], max_iter=max_iterationsGlobal[it])
        mlp = mlp.fit(X_train, y_train)
        acc += mlp.score(X_test, y_test)
    acc /= rkf.get_n_splits()
    return acc

def preProcessData(X, n):
    pca = PCA(n_components=n)
    pcaX = pca.fit_transform(X)
    return pcaX

def initAcc():
    acc = [0,0,0,0,0,0,0,0,0,0,0,0,0]
    return acc

def printHyperparams(act, solve, alph, it):
    f.write("HyperParams:\n")
    f.write("    Activation     =" + str(activationGlobal[act])+"\n")
    f.write("    Solver         =" + str(solverGlobal[solve])+"\n")
    f.write("    Alpha          =" + str(alphaGlobal[alph])+"\n")
    f.write("    Max_iterations =" + str(max_iterationsGlobal[it])+"\n")
    f.write("\n")

def printMax(myMax):
    f.write("\n")
    f.write("Values to get highest average score:\n")
    f.write("    accuracy      =" + str(myMax[0])+"\n")
    f.write("    activation    =" + str(myMax[1])+"\n")
    f.write("    solver        =" + str(myMax[2])+"\n")
    f.write("    alpha         =" + str(myMax[3])+"\n")
    f.write("    max_itertion  =" + str(myMax[4])+"\n")
    f.write("    n_comparisons =" + str(myMax[5])+"\n")

def main():
    X, y = readData()
    #acc, act, solve, alph, it, n_components
    myMax = [0]
    for act in range(3):
        for solve in range(3):
            for alph in range(3):
                for it in range(3):
                    acc = initAcc()
                    for n in range(1, 14):
                        for m in range(TRIALS):
                            acc[n-1] += mlpKFold(preProcessData(X, n), y, 2, act, solve, alph, it)

                    printHyperparams(act, solve, alph, it) 
                    for n in range(1, 14):
                        temp = acc[n-1] / TRIALS
                        f.write('accuracy of PCA(n_components=' + str(n) +") = " + str(temp)+"\n")
                        if myMax[0] < temp:
                            myMax = []
                            myMax.append(temp)
                            myMax.append(activationGlobal[act])
                            myMax.append(solverGlobal[solve])
                            myMax.append(alphaGlobal[alph])
                            myMax.append(max_iterationsGlobal[it])
                            myMax.append(n)
    printMax(myMax)
    f.close()


if __name__ == "__main__":
    main()
    

