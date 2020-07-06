#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <random>

#include "func.h"

/**
 * ReLU
 * @param i Element of Array
 * @return relu(i)
 */
double relu(double i) { return std::max(0.0, i); }

/**
 * loss
 * Compute the Loss
 * @param F Result from the output layer
 * @param y Labeled correct action
 * @return loss
 */
double loss(const Array2D F, const Array2D y) {
  /* Caclulate Loss */
  double loss = 0.0;
  double inner_sum = 0.0;
  for (size_t i = 0; i < y.height; i++) {
    inner_sum = 0.0;
    for (size_t k = 0; k < F.width; k++) {
      inner_sum += exp(F.data[i][k]);
    }
    loss += F.data[i][(int)y.data[i][0]] - log(inner_sum);
  }
  return loss * (-1.0 / F.height);
}

/**
 * cross_entropy
 * Computes the cross entropy
 * @param F Result from the output layer
 * @param y Labeled correct action
 * @return derivative
 */
Array2D cross_entropy(const Array2D F, const Array2D y) {
  Array2D dF = F;
  double selector = 0.0;
  double inner_sum = 0.0;
  /* Calculate dF */
  for (size_t i = 0; i < dF.height; i++) {
    for (size_t j = 0; j < dF.width; j++) {
      inner_sum = 0.0;
      selector = 0.0;
      for (size_t k = 0; k < dF.width; k++) {
        inner_sum += exp(F.data[i][k]);
      }
      if (j == (size_t)y.data[i][0]) {
        selector = 1.0;
      }
      dF.data[i][j] =
          (-1.0 / dF.height) * (selector - (exp(F.data[i][j]) / inner_sum));
    }
  }
  return dF;
}

/**
 * argmax
 * returns the index of the maximal element of the array
 * @param F Result from the output layer
 * @return index
 */
int argmax(const Array2D F) {
  auto result = std::max_element(F.data[0].begin(), F.data[0].end());
  return std::distance(F.data[0].begin(), result);
}

/**
 * standardize
 * Standardize the features in the array
 * @param input
 * @return standardized input
 */
Array2D standardize(const Array2D &input) {
  assert(input.width > 0);
  assert(input.height > 0);
  std::vector<double> sum;
  std::vector<double> mean;
  std::vector<double> stdev;
  sum.resize(input.width, 0.0);
  mean.resize(input.width, 0.0);
  stdev.resize(input.width, 0.0);
  Array2D ret = input;

  // Calculate Sums of Columns
  for (size_t i = 0; i < ret.height; i++) {
    for (size_t j = 0; j < ret.width; j++) {
      sum[j] += ret.data[i][j];
    }
  }

  // Compute Mean:
  for (size_t i = 0; i < mean.size(); i++) {
    mean[i] = sum[i] / (double)ret.height;
  }

  // Calculate Stdev
  for (size_t i = 0; i < ret.height; i++) {
    for (size_t j = 0; j < ret.width; j++) {
      stdev[j] += std::pow(ret.data[i][j] - mean[j], 2.0);
    }
  }
  for (auto &&i : stdev) {
    i = std::sqrt(i / (double)ret.height);
  }

  // Apply Standardization
  for (size_t i = 0; i < ret.height; i++) {
    for (size_t j = 0; j < ret.width; j++) {
      ret.data[i][j] -= mean[j];
      if (stdev[j] != 0.0) {
        ret.data[i][j] /= stdev[j];
      }
    }
  }
  return ret;
}

/**
 * normalize
 * Normalize columns in the array
 * @param input
 * @return normalized input
 */
Array2D normalize(const Array2D &input) {
  assert(input.width > 0);
  assert(input.height > 0);
  std::vector<double> sum;
  std::vector<double> min;
  std::vector<double> max;
  sum.resize(input.width, 0.0);
  min.resize(input.width, std::numeric_limits<double>::max());
  max.resize(input.width, std::numeric_limits<double>::lowest());
  Array2D ret = input;

  // Calculate Min & Max Features
  for (size_t i = 0; i < ret.height; i++) {
    for (size_t j = 0; j < ret.width; j++) {
      if (ret.data[i][j] < min[j]) {
        min[j] = ret.data[i][j];
      }
      if (ret.data[i][j] > max[j]) {
        max[j] = ret.data[i][j];
      }
    }
  }

  // Apply Normalization
  for (size_t i = 0; i < ret.height; i++) {
    for (size_t j = 0; j < ret.width; j++) {
      ret.data[i][j] = ret.data[i][j] - min[j];
      if (min[j] != max[j]) {
        ret.data[i][j] /= (max[j] - min[j]);
      }
    }
  }
  return ret;
}

/**
 * rescale
 * Rescale all the elemets in the array from [a0,a1] -> [b0, b1]
 * @param input
 * @param a0, a1 The source range
 * @param b0, b1 The destination range
 * @return scaled input
 */
Array2D rescale(const Array2D &input,
                double a0,
                double a1,
                double b0,
                double b1) {
  assert(input.width > 0);
  assert(input.height > 0);
  Array2D ret = input;
  double da = a1 - a0;
  double db = b1 - b0;
  // Apply rescaling
  for (size_t i = 0; i < ret.height; i++) {
    for (size_t j = 0; j < ret.width; j++) {
      ret.data[i][j] = (db*(ret.data[i][j]-a0)/da)+b0;
    }
  }
  return ret;
}
