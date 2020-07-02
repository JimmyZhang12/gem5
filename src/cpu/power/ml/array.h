#ifndef __ARRAY2D_H__
#define __ARRAY2D_H__

#include <iostream>
#include <ostream>
#include <vector>

#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

/**
 * Class Array2D
 *   2Dimensional Array Class
 */
class Array2D {
public:
  std::vector<std::vector<double>> data;
  size_t width;
  size_t height;

  Array2D();
  Array2D(size_t height,
          size_t width,
          double init = 0.0,
          bool identity = false);

  /**
   * operator*(const Array2D& other)
   * Multiply two arrays
   * Return Array2D
   */
  Array2D operator*(const Array2D &other);

  /**
   * operator*(const double& other)
   * Multiply array by scalar
   * Return Array2D
   */
  Array2D operator*(const double &other);

  /**
   * operator+(const Array& other)
   * Add Array and bias
   * Return Array
   */
  Array2D add_bias(const Array2D &bias);

  /**
   * operator+(const Array& other)
   * Add two arrays elementwise, must have same
   * dimensions
   * Return Array
   */
  Array2D operator+(const Array2D &other);

  /**
   * operator-(const Array& other)
   * Subtract two arrays elementwise, must have same
   * dimensions
   * Return Array
   */
  Array2D operator-(const Array2D &other);

  /**
   * transpose
   * multiplies the array by the identity matrix
   * @return transpose
   */
  Array2D transpose();

  /**
   * apply
   * Apply a function to the array element-wise
   * @return Array2D with applied function
   */
  Array2D apply(double func(double));

  /**
   * get_subset
   * return an array of dimensions y, x starting at
   * offset y_o, x_o
   * @param y height of new array
   * @param x width of new array
   * @param y_o y offset of new array
   * @param x_o x offset of new array
   * @return array subset
   */
  Array2D get_subset(size_t y, size_t x, size_t y_o, size_t x_o);

  /**
   * apply_subset
   * apply and array of dimensions y, x starting at
   * offset y_o, x_o
   * @param array Array to apply
   * @param y_o y offset of new array
   * @param x_o x offset of new array
   * @return array
   */
  void apply_subset(const Array2D &arr, size_t y_o, size_t x_o);

  /**
   * apply_shuffle
   * randomize the array, apply locally
   */
  void apply_shuffle();

  /**
   * operator<<
   * Print out the array in a nice format
   * @param ostream
   * @param arr Array to print
   * @return ostream
   */
  friend std::ostream &operator<<(std::ostream &out, const Array2D &arr) {
    out << "Shape: [" << arr.height << "," << arr.width << "]\n";
    for (auto i : arr.data) {
      for (auto j : i) {
        out << j << " ";
      }
      out << "\n";
    }
    return out;
  }

  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    if (version >= 0) {
      ar &data;
      ar &height;
      ar &width;
    }
  }
};

#endif // __ARRAY2D_H__
