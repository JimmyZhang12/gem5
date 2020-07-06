#ifndef __FUNC_H__
#define __FUNC_H__

#include "array.h"

/**
 * ReLU
 * @param i Element of Array
 * @return relu(i)
 */
double relu(double i);

/**
 * loss
 * Compute the Loss
 * @param F Result from the output layer
 * @param y Labeled correct action
 * @return loss
 */
double loss(const Array2D F, const Array2D y);

/**
 * cross_entropy
 * Computes the cross entropy
 * @param F Result from the output layer
 * @param y Labeled correct action
 * @return derivative
 */
Array2D cross_entropy(const Array2D F, const Array2D y);

/**
 * argmax
 * returns the index of the maximal element of the array
 * @param F Result from the output layer
 * @return index
 */
int argmax(const Array2D F);

/**
 * standardize
 * Standardize the features in the array
 * @param input
 * @return standardized input
 */
Array2D standardize(const Array2D &input);

/**
 * normalize
 * Normalize the features in the array
 * @param input
 * @return normalized input
 */
Array2D normalize(const Array2D &input);

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
                double b1);

#endif // __FUNC_H__
