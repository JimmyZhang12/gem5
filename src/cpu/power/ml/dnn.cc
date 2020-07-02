#include <algorithm>
#include <cmath>
#include <random>

#include "dnn.h"
#include "func.h"

/**
 * Constructor
 * Create a 3 Layer DNN
 * @param features
 * @param actions
 * @param hidden_dimension
 * @param hidden_layers
 * @param eta
 * @param init
 */
DNN::DNN(size_t features,
         size_t actions,
         size_t hidden_dim,
         size_t hidden_layers,
         double eta,
         double init) {
  input = Layer(features, hidden_dim, eta, init);
  for (size_t i = 0; i < hidden_layers; i++) {
    hidden.push_back(Layer(hidden_dim, hidden_dim, eta, init));
  }
  output = Layer(hidden_dim, actions, eta, init, true);
  this->features = features;
  this->actions = actions;
  this->hidden_dim = hidden_dim;
  this->hidden_layers = hidden_layers;
}

/**
 * eval
 * Evaluate the dnn
 * @param input
 * @return Predicted Action
 */
int DNN::eval(Array2D input) {
  Array2D A, F;
  A = this->input.forward(input);
  for (size_t i = 0; i < hidden.size(); i++) {
    A = hidden[i].forward(A);
  }
  F = this->output.forward(A);
  return argmax(F);
}

/**
 * train
 * Evaluate the classifier and train with Gradient
 * Descent
 * @param batch
 * @param action
 * @return loss
 */
double DNN::train(Array2D batch, Array2D actions) {
  double l = 0.0;
  Array2D A, F, dF, dA;
  A = input.forward(batch);
  for (size_t i = 0; i < hidden.size(); i++) {
    A = hidden[i].forward(A);
  }
  F = output.forward(A);

  l = loss(F, actions);
  dF = cross_entropy(F, actions);

  dA = output.reverse(dF);
  for (size_t i = hidden.size(); i > 0; i--) {
    dA = hidden[i - 1].reverse(dA);
  }
  input.reverse(dA);
  return l;
}
