import os
import random
import time
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from PIL import Image
from skimage.transform import resize
from sklearn.model_selection import train_test_split
from sklearn.decomposition import PCA
from sklearn.metrics import accuracy_score, classification_report
from sklearn.neighbors import KNeighborsClassifier
from sklearn.neural_network import MLPClassifier
from tqdm import tqdm
from KNN import KNN1,KNN3,NearestCentroid
#Pathing and loading the images
CLASSIFICATION_ROOT = './classification/'  # set your folder path

data_list = [] 
for root, dirs, files in os.walk(CLASSIFICATION_ROOT):
    for file in files:
        if file.lower().endswith(('.jpg', '.png', '.jpeg')):
            class_name = os.path.basename(os.path.dirname(os.path.join(root, file)))
            data_list.append({
                'Filename': file,
                'Filepath': os.path.join(root, file),
                'originalClassId': class_name
            })

#spreadsheet of all our samples
data_df = pd.DataFrame(data_list)
#debugging message
if data_df.empty:
    print("No images found. Check CLASSIFICATION_ROOT path.")
    exit()

unique_classes = sorted(data_df['originalClassId'].unique())#Gets all the unique labels {no doubles!No duplicates!}
class_to_idx = {cls: idx for idx, cls in enumerate(unique_classes)} #Each class name is converted into a numeric label {"corresponding number"}
data_df['label'] = data_df['originalClassId'].map(class_to_idx)  #This dictionary maps each class name to a numeric label f.e if our class name is cat,and the label is 1, it's cat:1

#PS:I used everywhere tqdm to see the time of the process cause i was afraid that my code was not working properly at first
X_data_list = []
print("Processing images:")
for _, row in tqdm(data_df.iterrows(), total=len(data_df)):
    img = Image.open(row['Filepath']).convert('L') #Converted to grayscale
    img_resized = resize(np.array(img), (32, 32)) #resized to 32 x 32
    X_data_list.append(img_resized.flatten()) #flattened into a 1D vector(32x32=1024 dimensions)
X_data = np.array(X_data_list) #(samples × features) all in 1 matrix
y_data = data_df['label'].values #corresponding labels/numerics

# PCA for Dimensionality Reduction with tqdm
NUM_COMPONENTS = 50
print("\nApplying PCA:") #message to see that everything works just fine
pca = PCA(n_components=NUM_COMPONENTS) #PCA compresses 1024-dimensional image vectors into 50 dimensions,which reduces training time and helps with visualisation.It also keeps the most important parts
X_data_reduced = pca.fit_transform(X_data)

# Train-Test Split
X_train, X_test, y_train, y_test = train_test_split(
    X_data_reduced, y_data, test_size=0.4, random_state=42, stratify=y_data
)

#Classifiers
classifiers = { #Ενδιάμεση εργασία classifiers
    "1-NN": KNN1(k=1),
    "3-NN": KNN3(k=3),
    "Nearest Centroid": NearestCentroid(),
    "Neural Network (MLP)": MLPClassifier(   #MLP classifier
        hidden_layer_sizes=(100,), max_iter=300, activation='relu', #One hidden layer with 100 neurons,max_iter=epochs {300 epochs maximum}, activation function=relu
        solver='adam', random_state=42, verbose=True #Adam:optimization algorithm used to update the weights of a neural network during training,verbose = “print detailed progress information while training"
    )
}

results = {}
#Records keeping
for name, clf in classifiers.items():
    print(f"\nTraining {name}...")
    start_train = time.time() #Time rec{gets the current system time in seconds.}
    clf.fit(X_train, y_train) #training step
    end_train = time.time() 
    
    print(f"Predicting {name}...")
    start_test = time.time() #Time start
    y_pred = clf.predict(X_test) #makes predictions for the test samples
    end_test = time.time()  #Time end {end-start}
    
    acc = accuracy_score(y_test, y_pred) #the percentage of test samples correctly classified,aka  Accuracy=#of correct predictions/#of predictions
    #Predictions are stored for later use.
    results[name] = {
        "Accuracy": acc,
        "Train Time (s)": end_train - start_train,
        "Test Time (s)": end_test - start_test,
        "Predictions": y_pred
    }
    print(f"{name}: Accuracy={acc:.4f}, Train={end_train-start_train:.2f}s, Test={end_test-start_test:.2f}s")#Printing the Dt=end-start for Training and testing and the accuracy

results_df = pd.DataFrame.from_dict({k: v for k,v in results.items()}, orient='index')
print("\nFINAL RESULTS")
print(results_df[['Accuracy','Train Time (s)','Test Time (s)']].to_string(float_format="%.4f"))

# -----------------------------
# Example Correct and Incorrect Predictions (for NN)
# -----------------------------
y_pred_nn = results["Neural Network (MLP)"]["Predictions"]
correct_indices = np.where(y_pred_nn == y_test)[0]
incorrect_indices = np.where(y_pred_nn != y_test)[0]

print(f"\nCorrectly classified samples: {len(correct_indices)}")
print(f"Misclassified samples: {len(incorrect_indices)}")

# Map test indices back to original dataframe indices
# Because train_test_split shuffles the data
_, X_test_idx = train_test_split(np.arange(len(X_data)), test_size=0.4, random_state=42, stratify=y_data)

# Show 2 correct examples
for idx in random.sample(list(correct_indices), min(2, len(correct_indices))):
    original_idx = X_test_idx[idx]
    img = Image.open(data_df.iloc[original_idx]['Filepath']).convert('L')
    plt.imshow(img, cmap='gray')
    plt.title(f"✅ Correct: {unique_classes[y_test[idx]]}")
    plt.axis('off')
    plt.show()

# Show 2 incorrect examples
for idx in random.sample(list(incorrect_indices), min(2, len(incorrect_indices))):
    original_idx = X_test_idx[idx]
    img = Image.open(data_df.iloc[original_idx]['Filepath']).convert('L')
    plt.imshow(img, cmap='gray')
    plt.title(f"❌ Predicted: {unique_classes[y_pred_nn[idx]]}\nTrue: {unique_classes[y_test[idx]]}")
    plt.axis('off')
    plt.show()