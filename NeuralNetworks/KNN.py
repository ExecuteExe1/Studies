import numpy as np
from collections import Counter
import pandas as pd
import numpy as np
import os

#distance function for the Euclidean distance
def distance(x1, x2):
    return np.sqrt(np.sum((x1 - x2) ** 2))
#Knn class
class KNN1:
    def __init__(self, k=1): #εδω συγκεκριμενα οριζω το k=1,constructor
        self.k = k

    def fit(self, X, y):    #stores training data
        self.X_train = X   #stores a matrix with the flattened images
        self.y_train = y   #Stores labels

    def predict(self, X_test):
        predictions = []
        for x in X_test:   #gives distances of samples from x
            # Vectorized distance calculation
            distances = np.linalg.norm(self.X_train - x, axis=1)
            nearest_indices = np.argsort(distances)[:self.k] #returns the neighbours? from nearest to furthest ,keeps only the closest neighbours to k {self.k}
            nearest_labels = self.y_train[nearest_indices] #we look at the labels of said nearest neighbours
            values, counts = np.unique(nearest_labels, return_counts=True) #counts how many times each label appears among the nearest neighbors.
            predictions.append(values[np.argmax(counts)])#adds on our predictions list the values of the most frequently picked label 
        return np.array(predictions)

class KNN3(KNN1): #nherits all methods and behavior from
    def __init__(self, k=3): #set the constructor and k=3
        super().__init__(k) #Calls the parent constructor with the corresponding k

class NearestCentroid:
    def __init__(self):
        self.centroids = None #dictionary
        self.classes = None #list of labels

    def fit(self, X, y):
        self.classes = np.unique(y) #gets all unique class labels
        self.centroids = {} #Creates empty dict.
        for c in self.classes: #loop from 0 to all our samples
            class_points = X[y == c] #extracts all training samples belonging to that class.
            self.centroids[c] = np.mean(class_points, axis=0) #Computes the mean of each feature (column) across all samples of a class,resulting in a single vector — the centroid of that class.
                            #f.e if we had [[1.2,3.0],[1.2,1.0]]then we have [[1.2,2.0]] 1
    def predict(self, X_test):
        predictions = []
        for x in X_test:
            distances = {c: np.linalg.norm(x - self.centroids[c]) for c in self.classes}
            predicted_class = min(distances, key=distances.get)
            predictions.append(predicted_class)

        return np.array(predictions)