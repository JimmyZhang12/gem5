#include <algorithm>
#include <cmath>
#include <random>

#include "perceptron.h"

Perceptron::Perceptron(size_t features, int init_range, double eta) {
  // Input vector is horizontal [1, features] weight vector needs to be
  // [features, 1]
  w = Array2D(features, 1, init_range);
  this->eta = eta;
}

double Perceptron::eval(Array2D input) const { return (input * w).data[0][0]; }

void Perceptron::train(Array2D input, bool correct) {
  if (correct) {
    w = w + (input.transpose() * eta);
  } else {
    w = w - (input.transpose() * eta);
  }
}

Classifier::Classifier(size_t actions,
                       size_t features,
                       int init_range,
                       double eta) {
  this->actions.resize(actions, Perceptron(features, init_range, eta));
}

int Classifier::eval(Array2D input) {
  std::vector<double> confidence;

  for (size_t i = 0; i < actions.size(); i++) {
    confidence.push_back(actions[i].eval(input));
  }

  auto result = std::max_element(confidence.begin(), confidence.end());
  return std::distance(confidence.begin(), result);
}

bool Classifier::train(Array2D input, Array2D label) {
  int predicted = eval(input);
  int action = (int)label.data[0][0];
  bool ret = true;
  if (predicted != action) {
    ret = false;
    actions[predicted].train(input, false);
    actions[action].train(input, true);
  }
  return ret;
}
