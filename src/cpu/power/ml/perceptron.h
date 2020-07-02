#ifndef __PERCEPTRON_H__
#define __PERCEPTRON_H__

#include <algorithm>
#include <cstdint>
#include <vector>

#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

#include "array.h"

class Perceptron {
  /** weights */
  Array2D w;

  /** Eta; training rate */
  double eta;

public:
  Perceptron() { eta = 0.0; };

  /**
   * Constructor
   * Create a signature perceptron
   * @param features The number of input features
   * @param init_range range for random init
   * @param eta training rate
   */
  Perceptron(size_t features, int init_range = 10, double eta = 0.1);

  /**
   * eval
   * Evaluate the perceptron
   * @param input
   * @return confidence
   */
  double eval(Array2D input) const;

  /**
   * train
   * @param input
   * @param correct
   */
  void train(Array2D input, bool correct);

  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    if (version >= 0) {
      ar &w;
      ar &eta;
    }
  }
};

class Classifier {
  std::vector<Perceptron> actions;

public:
  Classifier(){};

  /**
   * Constructor
   * Create a signature classifier
   * @param actions The number of actions to take
   * @param features The number of input features
   * @param init_range range for random init
   * @param eta training rate
   */
  Classifier(size_t actions,
             size_t features,
             int init_range = 10,
             double eta = 0.1);

  /**
   * eval
   * Evaluate the classifier
   * @return action taken
   */
  int eval(Array2D input);

  /**
   * train
   * Evaluate the classifier and train
   * @param input array
   * @param label
   * @return correct or incorrect
   */
  bool train(Array2D input, Array2D label);

  int get_actions() const { return (int)actions.size(); }

  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    if (version >= 0) {
      ar &actions;
    }
  }
};

#endif // __PERCEPTRON_H__
