# GestureRecogniser

This repository is part of the TU Delft CSE Research Project in 2023 — specifically, the project on creating TinyML-empowered Visible Light Sensing.

The repository is used to train and deploy a machine learning (ML) model to recognise gestures such as swipe left or double tap. The repository is used in combination with a data collection repository which was used to create the dataset used for training the ML models in this repository. It can be found [here](https://github.com/arnedebeer/CSE3000-DataCollection).

The repository consists of two parts:
- The [Model](Model) folder consists of all ML-related stuff. From loading a dataset, pre-processing data, and training and validating a model.
- The [GestureRecogniser](GestureRecogniser) folder consists of a proof-of-concept program made for the Arduino Nano 33. The program utilises various buffers to detect when to start gathering data and adjust activation thresholds on the fly. After all data for a gesture is collected, the data from each photodiode individually goes through a pre-processing pipeline and an inference is made.

## Preparing the repository
After cloning the repository, make sure all submodules have been initialised. The TensorFlow Lite library in the PlatformIO program may show some errors. 

To resolve these errors remove the ``examples``, ``peripherals`` and ``test_over_serial`` folders. Finally, modify ``src\tensorflow\lite\micro\micro_time.cpp`` to no longer include removed the ``peripherals/utility.h`` header file and make the ``GetCurrentTimeTicks()`` method return 0.

## Usage

To deploy a trained model to the microcontroller follow these steps:
- In ``notebook main.ipynb``, first specify what model to use, configure the training parameters, and then hit run all.
- After this is done, the TFLite model that is saved should be exported to C code. To do this perform the following command in a Linux shell ``xxd -i converted_model.tflite > model_data.cpp`` or ``xxd -i converted_model.tflite > ../GestureRecogniser/src/model/model_data.cpp`` to export the model immediately to the microcontroller program.
- Compile the PlatformIO microcontroller program and upload it to the microcontroller.
- When gestures are performed and inferences are made the microcontroller sends the results over the serial interface.