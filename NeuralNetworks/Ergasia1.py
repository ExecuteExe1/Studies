import pandas as pd
import numpy as np
import os
from PIL import Image
from skimage.transform import resize
from sklearn.model_selection import train_test_split
from sklearn.neighbors import KNeighborsClassifier, NearestCentroid
from sklearn.metrics import accuracy_score, classification_report
from sklearn.decomposition import PCA
from tqdm import tqdm
from collections import Counter
# from KNN import KNN1,KNN3,NearestCentroid
#Κανονικά είχα ΑΛΛΟ αρχειο ονοματι KNN με τον παρακατω κώδικα αλλα μας αφήνει να ανεβάσουμε ΜΟΝΟ 1 αρχειο..........
#-------------------KNN from Scratch!------------------

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

#Reading Data and basically the part of the FIRST Assignment.I started THIS first and then did the above^
# Root Dir
CLASSIFICATION_ROOT = './classification/'

print("Loading all images and labels from nested folders...\n")
data_list = []
#Recursively go through all subfolders
for root, dirs, files in os.walk(CLASSIFICATION_ROOT): #os.walk(path) recursively walks through all directories and subdirectories starting from the given root path
    for file in files:
        if file.lower().endswith(('.jpg', '.png', '.jpeg')):
            #The class name is the last folder name before the file
            class_name = os.path.basename(os.path.dirname(os.path.join(root, file)))
            data_list.append({
                'Filename': file,
                'Filepath': os.path.join(root, file),
                'originalClassId': class_name
            })

#Convert to DataFrame,ειναι σαν ενα πινακακι sql ουσιαστικα
data_df = pd.DataFrame(data_list)

if data_df.empty:
    print("Error: No images found.")
    exit()

print(f"Total images loaded: {len(data_df)}")
print(f"Unique classes found: {data_df['originalClassId'].nunique()}\n")
print("Classes:")
print(sorted(data_df['originalClassId'].unique()), "\n")


unique_classes = sorted(data_df['originalClassId'].unique()) #Access to all the UNIQUE classes+sorts em alphabetically
class_to_idx = {cls_name: idx for idx, cls_name in enumerate(unique_classes)} #Enumerates each class
data_df['label'] = data_df['originalClassId'].map(class_to_idx) #Replaces each class's name with its identification number{label},basically i create a numeric label?

print("Class mapping:")
print(class_to_idx, "\n")


X_data_list = []
print("Resizing and flattening images...")
for _, row in tqdm(data_df.iterrows(), total=len(data_df), desc="Processing images"):
    img = Image.open(row['Filepath']).convert('L')  # grayscale
    img_resized = resize(np.array(img), (32, 32))
    X_data_list.append(img_resized.flatten())

X_data = np.array(X_data_list)
y_data = data_df['label'].values

#Pca?
NUM_COMPONENTS = 50
pca = PCA(n_components=NUM_COMPONENTS)
X_data_reduced = pca.fit_transform(X_data)
print(f"\nPCA: {X_data.shape[1]} = {X_data_reduced.shape[1]} components\n")

#Data Splittin
X_train, X_test, y_train, y_test = train_test_split(
    X_data_reduced, y_data, test_size=0.4, random_state=42, stratify=y_data
)

print(f"Training samples: {X_train.shape[0]}")
print(f"Testing samples: {X_test.shape[0]}\n")

#Classifiers
classifiers = {
    "1-NN (k=1)": KNeighborsClassifier(n_neighbors=1),
    "3-NN (k=3)": KNeighborsClassifier(n_neighbors=3),
    "Nearest Centroid": NearestCentroid()
}

results = {}

for name, clf in classifiers.items():
    clf.fit(X_train, y_train)
    y_pred = clf.predict(X_test)
    acc = accuracy_score(y_test, y_pred)
    results[name] = acc
    print(f"{name}: Accuracy = {acc:.4f}")



#---Detailed Report-----
#best_name = max(results, key=results.get)
#best_model = classifiers[best_name]
#y_pred_best = best_model.predict(X_test)
#
#print(f"\nDetailed Report for Best Model ({best_name}):")
#print(classification_report(y_test, y_pred_best, target_names=unique_classes))


#--Custome Made----

print("\nNow evaluating our custom implementations...\n")
custom_classifiers = {
    "Custom 1-NN (k=1)": KNN1(k=1),
    "Custom 3-NN (k=3)": KNN3(k=3),
    "Custom Nearest Centroid": NearestCentroid()
}

custom_results = {}

for name, clf in custom_classifiers.items():
    clf.fit(X_train, y_train)
    y_pred = clf.predict(X_test)
    acc = accuracy_score(y_test, y_pred)
    custom_results[name] = acc
    print(f"{name}: Accuracy = {acc:.4f}")

# Find best-performing custom model
#best_custom_name = max(custom_results, key=custom_results.get)
#best_custom_model = custom_classifiers[best_custom_name]
#y_pred_best_custom = best_custom_model.predict(X_test)
#
#print(f"\nDetailed Report for Best Custom Model ({best_custom_name}):")
#print(classification_report(y_test, y_pred_best_custom, target_names=unique_classes))

