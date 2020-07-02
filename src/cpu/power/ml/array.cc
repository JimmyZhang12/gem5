#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>

#include "array.h"

Array2D::Array2D() {
  this->width = 0;
  this->height = 0;
}

Array2D::Array2D(size_t height, size_t width, double init, bool identity) {
  this->width = width;
  this->height = height;
  if (identity) {
    assert(height == width);
  }
  data.resize(height);
  for (size_t y = 0; y < data.size(); y++) {
    data[y].resize(width);
  }
  if (identity) {
    for (size_t y = 0; y < height; y++) {
      data[y][y] = 1.0;
    }
  } else {
    for (size_t y = 0; y < height; y++) {
      for (size_t x = 0; x < width; x++) {
        data[y][x] =
            (((double)(random() % 1000000) / 1000000.0) * 2.0 - 1.0) * init;
      }
    }
  }
}

Array2D Array2D::operator*(const Array2D &other) {
  assert(this->width == other.height);
  Array2D a = Array2D(this->height, other.width, 0.0);

  for (size_t i = 0; i < this->height; i++) {
    for (size_t j = 0; j < other.width; j++) {
      for (size_t k = 0; k < this->width; k++) {
        a.data[i][j] += this->data[i][k] * other.data[k][j]; // + b.data[j][0];
      }
    }
  }
  return a;
}

Array2D Array2D::operator*(const double &other) {
  Array2D a = Array2D(this->height, this->width, 0.0);

  for (size_t i = 0; i < this->height; i++) {
    for (size_t j = 0; j < this->width; j++) {
      a.data[i][j] = this->data[i][j] * other;
    }
  }
  return a;
}

Array2D Array2D::add_bias(const Array2D &bias) {
  assert(this->width == bias.height);
  Array2D a = *this;

  for (size_t i = 0; i < this->height; i++) {
    for (size_t j = 0; j < this->width; j++) {
      a.data[i][j] += bias.data[j][0];
    }
  }
  return a;
}

Array2D Array2D::operator+(const Array2D &other) {
  assert(this->height == other.height);
  assert(this->width == other.width);
  Array2D a = *this;

  for (size_t i = 0; i < this->height; i++) {
    for (size_t j = 0; j < this->width; j++) {
      a.data[i][j] += other.data[i][j];
    }
  }
  return a;
}

Array2D Array2D::operator-(const Array2D &other) {
  assert(this->height == other.height);
  assert(this->width == other.width);
  Array2D a = *this;

  for (size_t i = 0; i < this->height; i++) {
    for (size_t j = 0; j < this->width; j++) {
      a.data[i][j] -= other.data[i][j];
    }
  }
  return a;
}

Array2D Array2D::transpose() {
  Array2D transpose = Array2D(this->width, this->height, 0.0);
  for (size_t i = 0; i < this->height; i++) {
    for (size_t j = 0; j < this->width; j++) {
      transpose.data[j][i] = this->data[i][j];
    }
  }
  return transpose;
}

Array2D Array2D::apply(double func(double)) {
  Array2D a = *this;
  for (size_t i = 0; i < this->height; i++) {
    for (size_t j = 0; j < this->width; j++) {
      a.data[i][j] = func(this->data[i][j]);
    }
  }
  return a;
}

Array2D Array2D::get_subset(size_t y, size_t x, size_t y_o, size_t x_o) {
  assert(y + y_o <= this->height);
  assert(x + x_o <= this->width);
  Array2D ret(y, x, 0.0);
  for (size_t i = 0; i < y; i++) {
    for (size_t j = 0; j < x; j++) {
      ret.data[i][j] = this->data[i + y_o][j + x_o];
    }
  }
  return ret;
}

void Array2D::apply_subset(const Array2D &arr, size_t y_o, size_t x_o) {
  assert(arr.height + y_o <= this->height);
  assert(arr.width + x_o <= this->width);
  for (size_t i = 0; i < arr.height; i++) {
    for (size_t j = 0; j < arr.width; j++) {
      this->data[i + y_o][j + x_o] = arr.data[i][j];
    }
  }
}

void Array2D::apply_shuffle() {
  std::random_shuffle(this->data.begin(), this->data.end());
}
