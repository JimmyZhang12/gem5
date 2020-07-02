#ifndef __LAYER_H__
#define __LAYER_H__

#include <iostream>
#include <ostream>

#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/utility.hpp>

#include "array.h"

class Layer {
  Array2D W;
  Array2D b;

  // For Reverse
  Array2D Z;
  Array2D dW;
  Array2D db;
  Array2D dZ;
  Array2D A;
  Array2D dA;

  double eta;
  bool last_layer;

  /**
   * affine_forward
   * compute Z = (A x W) + b
   * @param A Input Array
   * @return Z
   */
  Array2D affine_forward(Array2D A);

  /**
   * relu_forward
   * apply relu to Z
   * @param Z Input array
   * @return ReLU(Z)
   */
  Array2D relu_forward(Array2D Z);

  /**
   * affine_reverse
   * Compute Gradient with respect to input
   * @param dZ output from relu_reverse
   * @return dA
   */
  Array2D affine_reverse(Array2D dZ);

  /**
   * relu_reverse
   * Reverse activation function
   * @param dA output from previous layer
   * @return dZ
   */
  Array2D relu_reverse(Array2D dA);

  /**
   * GD
   * Apply gradient descent to the weights and bias
   * @param Array
   * @param derivative
   * @return Updated Array
   */
  Array2D gd(Array2D X, Array2D dX);

public:
  Layer(){};

  Layer(size_t height,
        size_t width,
        double eta,
        double init,
        bool last_layer = false);

  Array2D forward(Array2D A);

  Array2D reverse(Array2D dA);

  friend std::ostream &operator<<(std::ostream &os, const Layer &l) {
#ifdef DEBUG
    os << "--------------------------------------------------\n";
    os << "W:\n"
       << l.W << "b:\n"
       << l.b << "dW:\n"
       << l.dW << "db:\n"
       << l.db << "dZ:\n"
       << l.dZ << "A:\n"
       << l.A << "dA:\n"
       << l.dA << "eta: " << l.eta << "; last_layer: " << l.last_layer << "\n";
#else
    os << "--------------------------------------------------\n";
    os << "W:\n" << l.W << "b:\n" << l.b;
#endif
    return os;
  }

  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    if (version >= 0) {
      ar &W;
      ar &b;
      ar &eta;
      ar &last_layer;
    }
  }
};

#endif // __LAYER_H__
