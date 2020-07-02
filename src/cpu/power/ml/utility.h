#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <string>

#include "dnn.h"
#include "perceptron.h"

enum preprocess_t {
  RAW,         // Use the Raw Test Data
  STANDARDIZE, // Standardize the Test Data
  NORMALIZE    // Normalize the Test Data
};

/**
 * Read Data
 * Read the test or training data from a CSV File, and apply a
 * preprocessing function to the data.
 *
 * The data is always expected to be:
 *    Label, Feature 0, Feature 1, ...
 *
 * @param fname File of input data
 * @param events The number of events to read
 * @param process
 * @return Dataset
 */
Array2D read_data(std::string fname, size_t events, preprocess_t process);

/**
 * train_dnn
 * Minibatch Train the DNN
 * @param dnn The network to train
 * @param training_data
 * @param epochs The number of training intervals
 * @param minibatch_size The size of the minibatch, 1 = sgd
 */
void train_dnn(DNN &dnn,
               Array2D &train_data,
               size_t epochs,
               size_t minibatch_size);

/**
 * test_dnn
 * Test the DNN
 * @param dnn The network to test
 * @param test_data, Array of Test Data
 */
void test_dnn(DNN &dnn, Array2D &test_data);

/**
 * train_classifer
 * Train the Perceptron Classifier
 * @param a Classifier to train
 * @param training_data
 * @param epochs The number of training intervals
 */
void train_classifier(Classifier &a, Array2D &train_data, size_t epochs);

/**
 * test classifier
 * test the perceptron classifer on the input data
 * @param a Classifier to train
 * @param training_data
 * @param epochs The number of training intervals
 */
void test_classifier(Classifier &a, Array2D &test_data);

/**
 * Boost Serialization of DNN and Classifier
 */
void save_classifier(Classifier &a, std::string fname);

void save_dnn(DNN &a, std::string fname);

void restore_classifier(Classifier &a, std::string fname);

void restore_dnn(DNN &a, std::string fname);

/**
 * dnn_unit_tests
 * Unit test some of the functionality of the Array2D and Layer Classes
 */
void dnn_unit_test();

#endif // __UTILITY_H__
