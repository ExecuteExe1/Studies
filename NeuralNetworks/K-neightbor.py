import os
import numpy as np
import cv2
from tqdm import tqdm
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder
from sklearn.neighbors import KNeighborsClassifier, NearestCentroid
from sklearn.metrics import accuracy_score, classification_report

# Path to your dataset
data_dir = "classification"  # folder containing subfolders like C1, C2, Stop, etc.

X = []
y = []

# Load images and labels
for label_name in os.listdir(data_dir):
    class_folder = os.path.join(data_dir, label_name)
    if not os.path.isdir(class_folder):
        continue

    for filename in os.listdir(class_folder):
        if filename.lower().endswith(('.jpg', '.jpeg', '.png')):
            img_path = os.path.join(class_folder, filename)
            img = cv2.imread(img_path)
            if img is None:
                continue
            img = cv2.resize(img, (32, 32))
            img = img.flatten() / 255.0  # flatten and normalize
            X.append(img)
            y.append(label_name)  # keep string labels

X = np.array(X)
y = np.array(y)

# Convert string labels to integers
le = LabelEncoder()
y_encoded = le.fit_transform(y)

# Split dataset
X_train, X_test, y_train, y_test = train_test_split(
    X, y_encoded, test_size=0.3, random_state=42, stratify=y_encoded
)

# Initialize classifiers
classifiers = {
    "KNN (k=1)": KNeighborsClassifier(n_neighbors=1),
    "KNN (k=3)": KNeighborsClassifier(n_neighbors=3),
    "Nearest Centroid": NearestCentroid()
}

# Train, predict, and print detailed reports
for name, clf in classifiers.items():
    clf.fit(X_train, y_train)
    y_pred = clf.predict(X_test)
    acc = accuracy_score(y_test, y_pred)
    print(f"\n=== {name} ===")
    print(f"Accuracy: {acc:.3f}")
    print(classification_report(y_test, y_pred, target_names=le.classes_))
