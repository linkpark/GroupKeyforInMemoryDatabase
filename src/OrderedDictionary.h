/*
 * =================================================================
 *
 *            Filename:    OrderedDictionary.h
 *
 *         Description:    the ordered dictionary struct, in which data
 *                  is organised in order
 *             Version:    v1.0
 *             Created:    2015-09-18 16:40
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */

#ifndef ORDEREDDICTIONARY_H
#define ORDEREDDICTIONARY_H

#include <cstring>
#include "BaseDictionary.h"

#include <glog/logging.h>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <vector>

template <typename T>
class OrderedDictionary : public BaseDictionary<T> {
 public:
  OrderedDictionary() : is_asc_order_(true), entry_begin_(0) {}

  explicit OrderedDictionary(size_t n) : is_asc_order_(true), entry_begin_(0) {
    value_list_.reserve(n * sizeof(T));
  }

  virtual ~OrderedDictionary() {}

  void AddValue(T value) { value_list_.push_back(value); }

  // core function
  T GetValueForValueId(value_id_t value_id) {
    // first we should minus the begin entry value
    /*if( value_id < entry_begin_ ) {
        throw std::out_of_range("In OrderedDictionary::GetValueForValueId ::
    invalid value id! " );
    }*/

    value_id = value_id - entry_begin_;

    if (!IsValueIdValid(value_id)) {
      LOG(ERROR)
          << "In OrderedDictionary::GetValueFor ValueId :: ivalid value id: "
          << value_id;
      LOG(ERROR) << "dict begin entry: " << entry_begin_;
      LOG(ERROR) << "dict size: " << Size();
      throw std::out_of_range(
          "In OrderedDictionary::GetValueForValueId :: invalid value id:");
    }

    return value_list_.at(value_id);
  }

  // binary search the vector
  value_id_t GetValueIdForValue(const T& value) {
    auto it = std::lower_bound(value_list_.begin(), value_list_.end(), value);
    if (it == value_list_.end()) {
      throw std::out_of_range("can't find the element");
    } else if (*it != value) {
      throw std::out_of_range("can't find the element");
    }

    return (it - value_list_.begin() + entry_begin_);
  }

  // find the value lower bound
  value_id_t FindLowerBoundForValue(const T& value) {
    auto it = std::lower_bound(value_list_.begin(), value_list_.end(), value);
    return (it - value_list_.begin() + entry_begin_);
  }

  value_id_t FindUpperBoundForValue(const T& value) {
    auto it = std::upper_bound(value_list_.begin(), value_list_.end(), value);
    return (it - value_list_.begin() + entry_begin_);
  }

  value_id_t FindValueIdForValue(const T& value) {
    return GetValueIdForValue(value);
  }

  // judge the value or value_id is legal
  bool IsValueIdValid(value_id_t value_id) {
    if (value_id > value_list_.size()) {
      return false;
    }

    return true;
  }

  bool ValueExists(const T& value) {
    return std::binary_search(value_list_.begin(), value_list_.end(), value);
  }

  const T GetSmallestValue() {
    if (is_asc_order_)
      return value_list_[0];
    else
      return value_list_[value_list_.size() - 1];
  }

  const T GetGreatestValue() {
    if (is_asc_order_)
      return value_list_[value_list_.size() - 1];
    else
      return value_list_[0];
  }

  void set_is_asc_order(bool order) { is_asc_order_ = order; }

  std::shared_ptr<BaseDictionary<T> > Copy() {
    std::shared_ptr<OrderedDictionary> copy_dic =
        std::make_shared<OrderedDictionary>();

    // the stl lib is real copy in memory
    copy_dic->value_list_ = value_list_;
    copy_dic->is_asc_order_ = is_asc_order_;
    copy_dic->entry_begin_ = entry_begin_;

    return copy_dic;
  }

  std::shared_ptr<OrderedDictionary<T> > Split(uint64_t begin, uint64_t end) {
    std::shared_ptr<OrderedDictionary<T> > ret =
        std::make_shared<OrderedDictionary<T> >();

    size_t real_begin = begin - entry_begin_;
    size_t real_end = end - entry_begin_;

    ret->value_list_.insert(ret->value_list_.begin(),
                            value_list_.begin() + real_begin,
                            value_list_.begin() + real_end);

    /*for( size_t i = real_begin; i < real_end ; ++i ) {
        ret->value_list_.push_back( value_list_[i] );
    }*/

    ret->is_asc_order_ = is_asc_order_;

    return ret;
  }

  void Reverse(size_t n) { value_list_.reverse(n * sizeof(T)); }

  size_t Size() const { return value_list_.size(); }

  void Print() {
    std::cout << "the dic is:";
    for (auto ix : value_list_) {
      std::cout << ix << " ";
    }

    std::cout << std::endl;
  }

  uint64_t Statistic() {
    uint64_t sum =
        sizeof(T) * value_list_.capacity() + sizeof(bool) + sizeof(uint64_t);

    return sum;
  }

  void set_entry_begin(uint64_t entry_begin) { entry_begin_ = entry_begin; }

  uint64_t entry_begin() const { return entry_begin_; }

  uint64_t GetEndEntry() const {
    return (value_list_.size() + entry_begin_ - 1);
  }

  void AppendDic(OrderedDictionary<T>& delta_dic) {
    value_list_.insert(value_list_.end(), delta_dic.value_list_.begin(),
                       delta_dic.value_list_.end());
  }

 public:
  void SerializeToString(std::string& result) {
    size_t rows = value_list_.size();

    result.append((char*)(&rows), sizeof(size_t));
    result.append((char*)(&entry_begin_), sizeof(uint64_t));
    result.append((char*)(&is_asc_order_), sizeof(bool));
    result.append((char*)(value_list_.data()), (rows * sizeof(T)));
  }

  void DeserializeFromString(std::string& data) {
    const char* cursor = data.data();
    size_t rows = 0;
    T tmp;

    memcpy(&rows, cursor, sizeof(size_t));
    cursor += sizeof(size_t);

    memcpy(&entry_begin_, cursor, sizeof(uint64_t));
    cursor += sizeof(uint64_t);

    memcpy(&is_asc_order_, cursor, sizeof(bool));
    cursor += sizeof(bool);

    value_list_.insert(value_list_.begin(), (T*)(cursor),
                       ((T*)(cursor) + rows));
  }

  // reload operator area
 public:
  T operator[](value_id_t value_id) { return GetValueForValueId(value_id); }

 private:
  uint64_t entry_begin_;
  bool is_asc_order_;
  std::vector<T> value_list_;
};

template <>
class OrderedDictionary<std::string> : public BaseDictionary<std::string> {
 public:
  OrderedDictionary() : is_asc_order_(true), entry_begin_(0) {}

  explicit OrderedDictionary(size_t n) : is_asc_order_(true), entry_begin_(0) {
    offset_.reserve(sizeof(uint64_t) * (n + 1));
  }

  virtual ~OrderedDictionary() {}

  void AddValue(std::string value) {
    if (offset_.size() == 0) {
      offset_.push_back(0);
    }

    uint64_t end_point = offset_.back();
    offset_.push_back(end_point + value.length());
    buffer_.append(value);
  }

  std::string GetValueForValueId(value_id_t value_id) {
    value_id = value_id - entry_begin_;

    if (!IsValueIdValid(value_id)) {
      LOG(ERROR)
          << "In OrderedDictionary::GetValueFor ValueId :: ivalid value id: "
          << value_id;
      LOG(ERROR) << "dict begin entry: " << entry_begin_;
      LOG(ERROR) << "dict size: " << Size();
      throw std::out_of_range(
          "In OrderedDictionary::GetValueForValueId :: invalid value id:");
    }

    uint64_t range = offset_[value_id + 1] - offset_[value_id];
    std::string ret(buffer_, offset_[value_id], range);

    return ret;
  }

  value_id_t GetValueIdForValue(const std::string& value) {
    value_id_t lower_bound = 0;
    value_id_t higher_bound = Size() - 1;
    value_id_t mid;

    while (lower_bound <= higher_bound) {
      mid = (lower_bound + higher_bound) / 2;
      std::string target_string = GetValueForValueId(mid);

      if (value == target_string) return mid;
      if (value < target_string) higher_bound = mid - 1;
      if (value > target_string) lower_bound = mid + 1;
    }

    throw std::runtime_error("do not find the value!");
  }

  value_id_t FindLowerBoundForValue(const std::string& value) {
    value_id_t lower_bound = 0, middle;
    value_id_t half, len;
    len = Size() - 1;

    while (len > 0) {
      half = len >> 1;
      middle = lower_bound + half;
      if (GetValueForValueId(middle) < value) {
        lower_bound = middle + 1;
        len = len - half - 1;
      }

      else
        len = half;
    }

    return lower_bound;
  }

  value_id_t FindUpperBoundForValue(const std::string& value) {
    value_id_t lower_bound = 0, len = Size() - 1;
    value_id_t half, middle;

    while (len > 0) {
      half = len >> 1;
      middle = lower_bound + half;
      if (GetValueForValueId(middle) > value)
        len = half;
      else {
        lower_bound = middle + 1;
        len = len - half - 1;
      }
    }

    if (GetValueForValueId(lower_bound) <= value) return lower_bound + 1;

    return lower_bound;
  }

  value_id_t FindValueIdForValue(const std::string& value) {
    return GetValueIdForValue(value);
  }

  bool ValueExists(const std::string& value) {
    try {
      GetValueIdForValue(value);
      return true;
    } catch (...) {
      return false;
    }
  }

  bool IsValueIdValid(value_id_t value_id) {
    if (value_id > (offset_.size() - 1)) {
      return false;
    }

    return true;
  }

  const std::string GetSmallestValue() {
    if (is_asc_order_)
      return GetValueForValueId((0 + entry_begin_));
    else
      return GetValueForValueId((Size() - 1 + entry_begin_));
  }

  const std::string GetGreatestValue() {
    if (is_asc_order_)
      return GetValueForValueId((Size() - 1 + entry_begin_));
    else
      return GetValueForValueId(0 + entry_begin_);
  }

  size_t Size() const { return (offset_.size() - 1); }

  void set_is_asc_order(bool order) { is_asc_order_ = order; }

  std::shared_ptr<BaseDictionary<std::string> > Copy() {
    std::shared_ptr<OrderedDictionary> copy_dic =
        std::make_shared<OrderedDictionary>();

    // the stl lib is real copy in memory
    copy_dic->offset_ = offset_;
    copy_dic->buffer_ = buffer_;
    copy_dic->is_asc_order_ = is_asc_order_;
    copy_dic->entry_begin_ = entry_begin_;

    return copy_dic;
  }

  std::shared_ptr<OrderedDictionary<std::string> > Split(uint64_t begin,
                                                         uint64_t end) {
    std::shared_ptr<OrderedDictionary<std::string> > ret =
        std::make_shared<OrderedDictionary<std::string> >();

    size_t real_begin = begin - entry_begin_;
    size_t real_end = end - entry_begin_;

    ret->buffer_.append(buffer_, offset_[real_begin],
                        offset_[real_end] - offset_[real_begin]);

    ret->offset_.insert(ret->offset_.begin(), (offset_.begin() + real_begin),
                        (offset_.begin() + real_end + 1));

    uint64_t begin_offset = ret->offset_.front();
    for (size_t i = 0; i < ret->offset_.size(); ++i) {
      ret->offset_[i] = ret->offset_[i] - begin_offset;
    }

    ret->is_asc_order_ = is_asc_order_;

    return ret;
  }

  void Reverse(size_t n) { offset_.reserve(n * sizeof(uint64_t)); }

  void Print() {
    std::cout << "the dic is:";
    for (size_t i = 0; i < Size(); ++i) {
      std::cout << GetValueForValueId(i) << " ";
    }

    std::cout << std::endl;
  }

  uint64_t Statistic() {
    uint64_t sum = offset_.capacity() * sizeof(uint64_t) + buffer_.capacity() +
                   sizeof(bool) + sizeof(uint64_t);

    return sum;
  }

  void set_entry_begin(uint64_t entry_begin) { entry_begin_ = entry_begin; }

  uint64_t entry_begin() const { return entry_begin_; }

  uint64_t GetEndEntry() const { return (Size() + entry_begin_ - 1); }

  void AppendDic(OrderedDictionary<std::string>& delta_dic) {
    buffer_.append(delta_dic.buffer_);
    uint64_t begin_offset = offset_.back();

    for (size_t i = 1; i < delta_dic.offset_.size(); ++i) {
      offset_.push_back(begin_offset + delta_dic.offset_[i]);
    }
  }

  void SerializeToString(std::string& result) {
    size_t offset_size = offset_.size();
    size_t string_len = buffer_.length();

    result.append((char*)(&offset_size), sizeof(size_t));
    result.append((char*)(&entry_begin_), sizeof(uint64_t));
    result.append((char*)(&is_asc_order_), sizeof(bool));
    result.append((char*)(offset_.data()), sizeof(uint64_t) * offset_size);
    result.append((char*)(&string_len), sizeof(size_t));
    result.append(buffer_);
  }

  void DeserializeFromString(std::string& data) {
    const char* cursor = data.data();
    size_t offset_size = 0;
    size_t string_len = 0;

    memcpy(&offset_size, cursor, sizeof(size_t));
    cursor += sizeof(size_t);

    memcpy(&entry_begin_, cursor, sizeof(uint64_t));
    cursor += sizeof(uint64_t);

    memcpy(&is_asc_order_, cursor, sizeof(bool));
    cursor += sizeof(bool);

    offset_.insert(offset_.begin(), (uint64_t*)(cursor),
                   ((uint64_t*)(cursor) + offset_size));
    cursor += sizeof(uint64_t) * offset_size;

    memcpy(&string_len, cursor, sizeof(size_t));
    cursor += sizeof(size_t);

    buffer_.append(cursor, string_len);
  }

  std::string operator[](value_id_t value_id) {
    return GetValueForValueId(value_id);
  }

 private:
  bool is_asc_order_;
  uint64_t entry_begin_;

  std::vector<uint64_t> offset_;
  std::string buffer_;
};

#endif
