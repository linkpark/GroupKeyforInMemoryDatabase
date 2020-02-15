/*
 * =================================================================
 *
 *            Filename:    BaseDictionary.h
 *
 *         Description:    define a serises of interface the dictionay
 *                  should implements
 *
 *             Version:    v1.0
 *             Created:    2015-09-18 16:00
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */
#ifndef BASEDICTIONARY_H_
#define BASEDICTIONARY_H_
#include "AbstractDictionary.h"

#include <stdint.h>
#include <memory>

typedef uint64_t value_id_t;

template <typename T>
class BaseDictionary : public AbstractDictionary {
 public:
  virtual ~BaseDictionary() {}

  virtual void AddValue(T value) = 0;

  // core function
  virtual T GetValueForValueId(value_id_t value_id) = 0;
  virtual value_id_t GetValueIdForValue(const T& value) = 0;
  virtual value_id_t FindValueIdForValue(const T& value) = 0;

  // judge the value or value_id is legal
  virtual bool IsValueIdValid(value_id_t value_id) = 0;
  virtual bool ValueExists(const T& value) = 0;

  virtual const T GetSmallestValue() = 0;
  virtual const T GetGreatestValue() = 0;

  virtual size_t Size() const = 0;

  virtual std::shared_ptr<BaseDictionary> Copy() = 0;
  virtual uint64_t Statistic() = 0;
};
#endif
