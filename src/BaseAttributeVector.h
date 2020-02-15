/*
 * =================================================================
 *
 *            Filename:    BaseAttributeVector.h
 *
 *         Description:    the base class of all attribute vector,
 *                      define one series common interface about the
 *                      vector.
 *             Version:
 *             Created:    2015-09-16 15:21
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */

#ifndef BASEATTRIBUTEVECTOR_H_
#define BASEATTRIBUTEVECTOR_H_

#include <iostream>
#include <memory>

template <typename T>
class BaseAttributeVector {
 public:
  typedef T value_type_t;

  virtual ~BaseAttributeVector() {}

  // get single value by row number
  virtual T Get(size_t row) const = 0;

  // set single value by row number
  virtual void Set(size_t row, T& value) = 0;

  virtual void Resize(size_t row_number) = 0;

  // clear the attribut vector content
  virtual void Clear() = 0;

  virtual size_t Capacity() const = 0;

  // make a deep copy of the base attribute vector
  virtual std::shared_ptr<BaseAttributeVector<T> > Copy() = 0;
};

#endif  // endif define
