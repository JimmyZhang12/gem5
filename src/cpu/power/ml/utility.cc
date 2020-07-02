#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>   // std::stringstream
#include <stdexcept> // std::runtime_error
#include <string>
#include <thread>
#include <vector>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "array.h"
#include "func.h"
#include "perceptron.h"
#include "utility.h"

/**
 * Read Data
 * Read the test or training data from a CSV File, and apply a * preprocessing
 * function to the data.
 *
 * The data is always expected to be:
 *    Label, Feature1, Feature2, Feature3, ...
 *
 * @param fname File of input data
 * @param features The number of features in the dataset
 * @param process
 * @return Dataset
 */
Array2D read_data(std::string fname, size_t features, preprocess_t process) {
  std::cout << "Reading vectors from " << fname << "\n";
  Array2D ret;
  std::string file_line;
  std::string col_value;
  std::ifstream infile(fname);

  if (!infile.is_open()) {
    throw std::runtime_error("Could not open file");
  }

  size_t i = 0;
  while (infile.good()) {
    getline(infile, file_line);
    std::stringstream ss(file_line);

    // checking if its an empty line
    if (!(std::getline(ss, col_value, ','))) {
      continue;
    }

    // Create a New Entry:
    ret.data.push_back(std::vector<double>());
    ret.data[i].push_back((double)std::stoi(col_value));

    size_t count = 0;
    while (std::getline(ss, col_value, ',')) {
      if (count >= features) {
        break;
      }
      count++;
      ret.data[i].push_back((double)std::stoi(col_value));
    }
    i++;
    if (count != features) {
      throw std::runtime_error(
          "Number of Events specified is less than the number that exist");
    }
  }
  infile.close();
  if (ret.data.size()) {
    ret.width = ret.data[0].size();
  }
  ret.height = ret.data.size();

  Array2D temp;

  // Apply input formatting
  switch (process) {
    case RAW: {
      // Do nothing
      break;
    }
    case STANDARDIZE: {
      temp = ret.get_subset(ret.height, ret.width - 1, 0, 1);
      temp = standardize(temp);
      ret.apply_subset(temp, 0, 1);
      break;
    }
    case NORMALIZE: {
      temp = ret.get_subset(ret.height, ret.width - 1, 0, 1);
      temp = normalize(temp);
      ret.apply_subset(temp, 0, 1);
      break;
    }
    default: {
      break;
    }
  }

  std::cout << "Read " << i << " vectors from file: " << fname << "\n";
  return ret;
}

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
               size_t minibatch_size) {
  Array2D input;
  Array2D label;
  int int_label;
  double loss = 0;
  double t_correct = 0;
  double t_total = 0;
  std::vector<double> correct;
  std::vector<double> total;
  correct.resize(dnn.get_actions(), 0.0);
  total.resize(dnn.get_actions(), 0.0);
  for (size_t i = 0; i < epochs; i++) {
    train_data.apply_shuffle();
    input =
        train_data.get_subset(train_data.height, train_data.width - 1, 0, 1);
    label = train_data.get_subset(train_data.height, 1, 0, 0);
    for (size_t j = 0; j < correct.size(); j++) {
      correct[j] = 0.0;
      total[j] = 0.0;
    }
    t_correct = 0.0;
    t_total = 0.0;
    loss = 0.0;
    for (size_t j = 0; j < train_data.height - minibatch_size;
         j += minibatch_size) {
      loss += dnn.train(input.get_subset(minibatch_size, input.width, j, 0),
                        label.get_subset(minibatch_size, label.width, j, 0));
    }
    for (size_t j = 0; j < train_data.height; j++) {
      int_label = (int)label.get_subset(1, label.width, j, 0).data[0][0];
      if (int_label == dnn.eval(input.get_subset(1, input.width, j, 0))) {
        t_correct += 1.0;
        correct[int_label] += 1.0;
      }
      total[int_label] += 1.0;
      t_total += 1.0;
    }
    std::cout << i << "," << loss << "," << t_correct << "," << t_total << ","
              << t_correct / t_total;
    for (size_t k = 0; k < correct.size(); k++) {
      std::cout << "," << correct[k] / total[k];
    }
    std::cout << "\n";
  }
}

/**
 * train_dnn
 * Test the DNN
 * @param dnn The network to test
 * @param test_data, Array of Test Data
 */
void test_dnn(DNN &dnn, Array2D &test_data) {
  Array2D input;
  Array2D label;
  int int_label;
  double t_correct = 0;
  double t_total = 0;
  std::vector<double> correct;
  std::vector<double> total;
  correct.resize(dnn.get_actions(), 0.0);
  total.resize(dnn.get_actions(), 0.0);
  test_data.apply_shuffle();
  input = test_data.get_subset(test_data.height, test_data.width - 1, 0, 1);
  label = test_data.get_subset(test_data.height, 1, 0, 0);
  correct.resize(dnn.get_actions(), 0.0);
  total.resize(dnn.get_actions(), 0.0);
  t_correct = 0.0;
  t_total = 0.0;
  for (size_t j = 0; j < test_data.height; j++) {
    int_label = (int)label.get_subset(1, label.width, j, 0).data[0][0];
    if (int_label == dnn.eval(input.get_subset(1, input.width, j, 0))) {
      t_correct += 1.0;
      correct[int_label] += 1.0;
    }
    total[int_label] += 1.0;
    t_total += 1.0;
  }
  std::cout << "Test DNN " << t_correct << "," << t_total << ","
            << t_correct / t_total;
  for (size_t k = 0; k < correct.size(); k++) {
    std::cout << "," << correct[k] / total[k];
  }
  std::cout << "\n";
}

/**
 * train_classifer
 * Train the Perceptron Classifier
 * @param a Classifier to train
 * @param training_data
 * @param epochs The number of training intervals
 */
void train_classifier(Classifier &a, Array2D &train_data, size_t epochs) {
  Array2D input;
  Array2D label;
  int int_label;
  double t_correct = 0;
  double t_total = 0;
  std::vector<double> correct;
  std::vector<double> total;
  correct.resize(a.get_actions(), 0.0);
  total.resize(a.get_actions(), 0.0);
  for (size_t i = 0; i < epochs; i++) {
    train_data.apply_shuffle();
    input =
        train_data.get_subset(train_data.height, train_data.width - 1, 0, 1);
    label = train_data.get_subset(train_data.height, 1, 0, 0);
    t_correct = 0.0;
    t_total = 0.0;
    for (size_t j = 0; j < correct.size(); j++) {
      correct[j] = 0.0;
      total[j] = 0.0;
    }
    for (size_t j = 0; j < train_data.height; j++) {
      int_label = (int)label.get_subset(1, label.width, j, 0).data[0][0];
      if (a.train(input.get_subset(1, input.width, j, 0),
                  label.get_subset(1, label.width, j, 0))) {
        t_correct += 1.0;
        correct[int_label] += 1.0;
      }
      total[int_label] += 1.0;
      t_total += 1.0;
    }
    std::cout << i << "," << t_correct << "," << t_total << ","
              << t_correct / t_total;
    for (size_t k = 0; k < correct.size(); k++) {
      std::cout << "," << correct[k] / total[k];
    }
    std::cout << "\n";
  }
  std::cout << "Train Classifier " << t_correct << "," << t_total << ","
            << t_correct / t_total;
  for (size_t k = 0; k < correct.size(); k++) {
    std::cout << "," << correct[k] / total[k];
  }
  std::cout << "\n";
}

/**
 * test classifier
 * test the perceptron classifer on the input data
 * @param a Classifier to train
 * @param training_data
 * @param epochs The number of training intervals
 */
void test_classifier(Classifier &a, Array2D &test_data) {
  Array2D input;
  Array2D label;
  int int_label;
  double t_correct = 0;
  double t_total = 0;
  std::vector<double> correct;
  std::vector<double> total;
  correct.resize(a.get_actions(), 0.0);
  total.resize(a.get_actions(), 0.0);
  input = test_data.get_subset(test_data.height, test_data.width - 1, 0, 1);
  label = test_data.get_subset(test_data.height, 1, 0, 0);
  for (size_t j = 0; j < test_data.height; j++) {
    int_label = (int)label.get_subset(1, label.width, j, 0).data[0][0];
    if (int_label == a.eval(input.get_subset(1, input.width, j, 0))) {
      t_correct += 1.0;
      correct[int_label] += 1.0;
    }
    total[int_label] += 1.0;
    t_total += 1.0;
  }
  std::cout << "Test Classifier " << t_correct << "," << t_total << ","
            << t_correct / t_total;
  for (size_t k = 0; k < correct.size(); k++) {
    std::cout << "," << correct[k] / total[k];
  }
  std::cout << "\n";
}

void save_classifier(Classifier &a, std::string fname) {
  std::ofstream ofs(fname.c_str());
  boost::archive::text_oarchive oa(ofs);
  oa << a;
}

void save_dnn(DNN &a, std::string fname) {
  std::ofstream ofs(fname.c_str());
  boost::archive::text_oarchive oa(ofs);
  oa << a;
}

void restore_classifier(Classifier &a, std::string fname) {
  std::ifstream ifs(fname.c_str());
  boost::archive::text_iarchive ia(ifs);
  ia >> a;
}

void restore_dnn(DNN &a, std::string fname) {
  std::ifstream ifs(fname.c_str());
  boost::archive::text_iarchive ia(ifs);
  ia >> a;
}

/**
 * dnn_unit_tests
 * Unit test some of the functionality of the Array2D and Layer Classes
 */
void dnn_unit_test() {
#ifdef DEBUG
  int i = 0;

  /*
   * Test the Default Contructor of Array2D
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": Create Array2D\n";
  std::cout << "-----------------------------------------------------\n";
  Array2D A = Array2D();
  std::cout << A << "\n";
  std::cout << "\n\n";

  /*
   * Test the NxM constructor & random Init
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": Create Array2D; Height=4, Width=6\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(4, 6);
  std::cout << A << "\n";
  std::cout << "\n\n";

  /*
   * Test Multiplication of Array and Scalar
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": Multiply Array2D*double\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(4, 4, 1.0);
  Array2D B = A * 100.0;
  std::cout << A << "\n";
  std::cout << B << "\n";
  std::cout << "\n\n";

  /*
   * Test Multiplication of Array and Array
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": Multiply Array2D 2x2, 2x2\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(2, 2, 1.0);
  B = Array2D(2, 2, 1.0);
  Array2D C = A * B;
  std::cout << A << "\n";
  std::cout << B << "\n";
  std::cout << C << "\n";
  std::cout << "\n\n";

  /*
   * Test Multiplication of Array and Array
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": Multiply Array2D 2x5, 5x2\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(2, 5, 1.0);
  B = Array2D(5, 2, 1.0);
  C = A * B;
  std::cout << A << "\n";
  std::cout << B << "\n";
  std::cout << C << "\n";
  std::cout << "\n\n";

  /*
   * Test identity matrix
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": Identity 5x5\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(5, 5, 0.0, true);
  std::cout << A << "\n";
  std::cout << "\n\n";

  /*
   * Test Transpose of Array and Array
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": Transpose Array2D 2x5\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(2, 5, 1.0);
  B = A.transpose();
  std::cout << A << "\n";
  std::cout << B << "\n";
  std::cout << "\n\n";

  /*
   * Test Addition of Array and Bias Vector
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": Add bias 2x5, 5x1\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(2, 5, 1.0);
  B = Array2D(5, 1, 1.0);
  C = A.add_bias(B);
  std::cout << A << "\n";
  std::cout << B << "\n";
  std::cout << C << "\n";
  std::cout << "\n\n";

  /*
   * Test Subtraction of Array and Array
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": Subtract Array2D 8x8, 8x8\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(8, 8, 1.0);
  B = A;
  C = A - B;
  std::cout << A << "\n";
  std::cout << B << "\n";
  std::cout << C << "\n";
  std::cout << "\n\n";

  /*
   * Test Addition of Array and Array
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": Subtract Array2D 8x8, 8x8\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(8, 8, 1.0);
  B = A;
  C = A + B;
  std::cout << A << "\n";
  std::cout << B << "\n";
  std::cout << C << "\n";
  std::cout << "\n\n";

  /*
   * Test ReLU Function
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": Array2D Apply relu()\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(4, 4, 10.0);
  B = A.apply(relu);
  std::cout << A << "\n";
  std::cout << B << "\n";
  std::cout << "\n\n";

  /*
   * Test Argmax()
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": argmax(Array2D)\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(1, 10, 10.0);
  std::cout << A << "\n";
  std::cout << argmax(A) << "\n";
  std::cout << "\n\n";

  /*
   * Test normalize
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": normalize(Array2D) 1x10\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(1, 10, 10.0);
  std::cout << A << "\n";
  std::cout << normalize(A) << "\n";
  std::cout << "\n\n";

  /*
   * Test normalize
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": normalize(Array2D) 10x1\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(10, 1, 10.0);
  std::cout << A << "\n";
  std::cout << normalize(A) << "\n";
  std::cout << "\n\n";

  /*
   * Test normalize
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": normalize(Array2D) 8x8\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(8, 8, 10.0);
  std::cout << A << "\n";
  std::cout << normalize(A) << "\n";
  std::cout << "\n\n";

  /*
   * Test standardize
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": standardize(Array2D) 1x10\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(1, 10, 10.0);
  std::cout << A << "\n";
  std::cout << standardize(A) << "\n";
  std::cout << "\n\n";

  /*
   * Test standardize
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": standardize(Array2D) 10x1\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(10, 1, 10.0);
  std::cout << A << "\n";
  std::cout << standardize(A) << "\n";
  std::cout << "\n\n";

  /*
   * Test standardize
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": standardize(Array2D) 8x8\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(8, 8, 10.0);
  std::cout << A << "\n";
  std::cout << standardize(A) << "\n";
  std::cout << "\n\n";

  /*
   * Test get_subset
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": get_subset 8x8 -> 8x7\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(8, 8, 10.0);
  B = A.get_subset(8, 7, 0, 1);
  std::cout << A << "\n";
  std::cout << B << "\n";
  std::cout << "\n\n";

  /*
   * Test get_subset
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": get_subset 8x8 -> 8x1\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(8, 8, 10.0);
  B = A.get_subset(8, 1, 0, 0);
  std::cout << A << "\n";
  std::cout << B << "\n";
  std::cout << "\n\n";

  /*
   * Test shuffle
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": shuffle 8x4\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(8, 4, 10.0);
  std::cout << A << "\n";
  A.apply_shuffle();
  std::cout << A << "\n";
  std::cout << "\n\n";

  /*
   * Test apply subset
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": apply_subset 8x4\n";
  std::cout << "-----------------------------------------------------\n";
  A = Array2D(8, 4, 10.0);
  B = A.get_subset(8, 3, 0, 1);
  B = normalize(B);
  std::cout << A << "\n";
  A.apply_subset(B, 0, 1);
  std::cout << A << "\n";
  std::cout << "\n\n";

  /*
   * Test read_file, raw
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": read_file, RAW\n";
  std::cout << "-----------------------------------------------------\n";
  A = read_data("./unit_test/input.csv", 4, RAW);
  std::cout << A << "\n";
  std::cout << "\n\n";

  /*
   * Test read_file, STANDARDIZE
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": read_file, STANDARDIZE\n";
  std::cout << "-----------------------------------------------------\n";
  A = read_data("./unit_test/input.csv", 4, STANDARDIZE);
  std::cout << A << "\n";
  std::cout << "\n\n";

  /*
   * Test read_file, NORMALIZE
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": read_file, NORMALIZE\n";
  std::cout << "-----------------------------------------------------\n";
  A = read_data("./unit_test/input.csv", 4, NORMALIZE);
  std::cout << A << "\n";
  std::cout << "\n\n";

  /*
   * Test Classifer Train
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": classifier train\n";
  std::cout << "-----------------------------------------------------\n";
  A = read_data("./unit_test/input.csv", 17, NORMALIZE);
  std::cout << A << "\n";
  Classifier classifier = Classifier(2, A.width - 1);
  train_classifier(classifier, A, 1000);
  std::cout << "\n\n";

  /*
   * Test Classifer Test
   */
  i++;
  std::cout << "-----------------------------------------------------\n";
  std::cout << "Test " << i << ": classifier test\n";
  std::cout << "-----------------------------------------------------\n";
  test_classifier(classifier, A);
  std::cout << "\n\n";
#endif
}
