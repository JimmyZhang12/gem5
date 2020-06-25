/*
 * Copyright (c) 2020, University of Illinois
 * All rights reserved
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2004-2005 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Andrew Smith
 */

/**
 * Bloomfilter constructor, initializes the offset array and the table to
 * false.
 * @param n The number of hashes to perform on the input object
 * @param size The size of the table
 * @param seed The seed for srand() for generating the n hashes
 * @return None
 */
template<class T>
Bloomfilter<T>::Bloomfilter(unsigned int n, unsigned int size, unsigned int seed) {
  std::srand(seed);
  offset.resize(n, 0);
  table.resize(size, false);
  for(size_t i = 0; i < offset.size(); i++) {
    offset[i] = std::rand();
  }
}

/**
 * Bloomfilter find. Checks if the object is in the filter. Due to the
 * nature of the bloom filter there is a chance of a false positive.
 * @param obj Item to be hashed
 * @return True if the item is in the table
 */
template<class T>
bool Bloomfilter<T>::find(const T obj) const {
  if(table.size() == 0 || offset.size() == 0) {
    return false;
  }
  bool retval = true;
  int idx = 0;
  for(size_t i = 0; i < offset.size(); i++) {
    idx = h(offset[i], obj) % table.size();
    retval &= table[idx];
  }
  return retval;
}

/**
 * Bloomfilter insert. Adds the object to the filter.
 * @param obj Item to be hashed
 */
template<class T>
void Bloomfilter<T>::insert(const T obj) {
  if(table.size() == 0) {
    return;
  }
  int idx = 0;
  for(size_t i = 0; i < offset.size(); i++) {
    idx = h(offset[i], obj) % table.size();
    table[idx] = true;
  }
}

/**
 * resize, clears out the bloom filter and resets the seed, n hash ways and the
 * size of the underlying table.
 * @param n The number of hashes to perform on the input object
 * @param size The size of the table
 * @param seed The seed for srand() for generating the n hashes
 * @return None
 */
template<class T>
void Bloomfilter<T>::resize(unsigned int n, unsigned int size, unsigned int seed) {
  clear();
  std::srand(seed);
  offset.resize(n, 0);
  table.resize(size, false);
  for(size_t i = 0; i < offset.size(); i++) {
    offset[i] = std::rand();
  }
}

/**
 * Reset the contents of the bloom filter, as it is not possible to erase a
 * specific item.
 */
template<class T>
void Bloomfilter<T>::clear() {
  std::replace(table.begin(), table.end(), true, false);
}

/**
 * Bloomfilter hash function, hashes the object n ways with the std::hash
 * function. Uses a different "offset" for hash.
 *    https://stackoverflow.com/a/7222201/916549
 *    https://www.boost.org/doc/libs/1_35_0/libs/functional/hash/examples/point.cpp
 * @param val Item to be hashed
 * @param offset Int prepended to the hash for the n-way hash
 * @return Hashed value
 */
template<class T>
size_t Bloomfilter<T>::h(const int offset, const T& val) const {
  size_t seed = 0;
  std::hash<int> hash1;
  std::hash<T> hash2;
  seed ^= hash1(offset) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  seed ^= hash2(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  return seed;
}

/**
 * Hash function for std::vector<T>. Note that T must also have a hash
 * function if it is not part of the Standard specializations for basic
 * types. Combines the hashes of all the elements.
 *    https://stackoverflow.com/a/7222201/916549
 *    https://www.boost.org/doc/libs/1_35_0/libs/functional/hash/examples/point.cpp
 * @param k Vector to be hashed
 * @return Hashed value
 */
namespace std {
  template <class T>
  struct hash<vector<T>> {
    size_t operator()(const std::vector<T>& k) const {
      hash<T> h;
      size_t seed = 0;
      for(auto it = k.begin(); it != k.end(); it++) {
        seed ^= h(*it) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }
      return seed;
    }
  };
}
