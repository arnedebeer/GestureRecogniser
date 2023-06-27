import itertools
from os import path
import matplotlib.pyplot as plt
import numpy as np

def plot_data_as_image(feature: np.ndarray, label: str = "Photodiode Signals", reshape: bool = True):
    fig, ax = plt.subplots(1)

    # Turn off y-axis ticks and tick labels
    ax.set_yticks([0, 1, 2])
    # ax.set_yticklabels([])

    # Reshape feature to be a 2D array
    if reshape:
        feature = np.reshape(feature, (100, 3))

    plt.imshow(feature.squeeze().transpose(), cmap='Greys_r', aspect=10)
    
    ax.set_yticklabels(["Red", "Green", "Blue"])

    # plt.imshow(feature.squeeze())
    plt.colorbar(orientation='horizontal', fraction=0.10, shrink=0.4)
    plt.title(label)
    plt.show()

def plot_data_as_graph(feature: np.ndarray, title: str= "Photodiode Signals", reshape: bool = True):
    if reshape:
        feature = np.reshape(feature, (100, 3))
    
    time_vals = np.arange(0, feature.shape[0])

    plt.plot(time_vals, feature[:, 0], 'r')
    plt.plot(time_vals, feature[:, 1], 'g')
    plt.plot(time_vals, feature[:, 2], 'b')
    plt.title(title)
    plt.xlabel("Sample")
    plt.ylabel("Reading")
    plt.legend()
    plt.show()

def plot_loss_and_accuracy(loss, accuracy):
    # History should contain the keys "loss", and "sparse_categorical_accuracy"
    plt.plot(loss, label="Loss")
    plt.plot(accuracy, label="Accuracy")
    plt.title("Loss and Accuracy")
    plt.xlabel("Epoch")
    plt.ylabel("Value")
    plt.legend()
    plt.show()

def plot_confusion_matrix(cm: np.ndarray, classes: list, normalize: bool=True, title: str="Confusion Matrix", cmap=plt.cm.Blues):
    """
    This function prints and plots the confusion matrix.
    Normalization can be applied by setting `normalize=True`.
    Source: https://scikit-learn.org/stable/auto_examples/model_selection/plot_confusion_matrix.html
    """
    if normalize:
        cm = cm.astype('float') / cm.sum(axis=1)[:, np.newaxis]
        print("Normalised confusion matrix")
    else:
        print('Confusion matrix, without normalization')

    plt.imshow(cm, interpolation='nearest', cmap=cmap)
    plt.title(title)
    plt.colorbar()
    tick_marks = np.arange(len(classes))
    plt.xticks(tick_marks, classes, rotation=45, ha="right")
    plt.yticks(tick_marks, classes)

    fmt = '.2f' if normalize else 'd'
    thresh = cm.max() / 2.

    # Print the actual numbers in the confusion matrix
    for i, j in itertools.product(range(cm.shape[0]), range(cm.shape[1])):
        plt.text(j, i, f"{format(cm[i, j], fmt)}",
                 horizontalalignment="center",
                 color="white" if cm[i, j] > thresh else "black",
                 fontsize=8)

    # plt.tight_layout()
    plt.ylabel('True label')
    plt.xlabel('Predicted label')
    plt.show()