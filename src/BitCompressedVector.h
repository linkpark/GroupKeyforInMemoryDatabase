/*
 * =================================================================
 *
 *            Filename:    BitCompressedVector.h
 *
 *         Description:    this class is used to compress the vector with
 *                      fixed length unit, especially the number.
 *
 *             Version:    v 1.0
 *             Created:    2015-09-16 15:37
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */

#ifndef BITCOMPRESSEDVECTOR_H_
#define BITCOMPRESSEDVECTOR_H_

#include <glog/logging.h>
#include "BaseAttributeVector.h"

#include <stdint.h>
#include <string.h>

#include <cmath>
#include <stdexcept>

template <typename T>
class BitCompressedVector : public BaseAttributeVector<T> {
 private:
  typedef uint64_t storage_unit_t;

 public:
  explicit BitCompressedVector(size_t rows, size_t bits = 0)
      : data_(0), rows_(rows), bits_(bits) {
    // if the bits is unset, then behave like a normal vector
    if (0 == bits) {
      bits_ = sizeof(T) * 8;
    }

    AllocData();
  }

  BitCompressedVector() : data_(0), rows_(0), size_(0), blocks_(0), bits_(0) {}

  virtual ~BitCompressedVector() {
    if (data_ != NULL) {
      Clear();
    }
  }

  // get single value by row number
  T Get(size_t row) const {
    if (row > rows_) {
      LOG(ERROR) << "In BitCompressedVector::Get(), beyond upper bound get row "
                 << row << "- actual size " << rows_ << std::endl;
      throw std::runtime_error(
          "In BitCompressedVector::Get(), beyond uppder bound!");
    }

    T result;

    auto offset = BlockOffset(row);
    auto block = BlockPosition(row) + offset / bit_width_;

    auto col_offset = offset % bit_width_;

    uint64_t bounds = bit_width_ - col_offset;
    uint64_t base_mask = (1ull << bits_) - 1ull;

    result = ((data_[block] >> col_offset) & base_mask);

    // deal with the bound situation
    if (bounds < bits_) {
      col_offset = bits_ - bounds;
      base_mask = (1ull << col_offset) - 1ull;
      result |= ((base_mask & data_[block + 1]) << bounds);
    }

    return result;
  }

  // set single value by row number
  void Set(size_t row, T& value) {
    if (row > rows_) {
      LOG(ERROR) << "In BitCompressedVector::Set(), beyond upper bound set row "
                 << row << "- actual size " << rows_ << std::endl;
      throw std::runtime_error(
          "In BitCompressedVector::Set(), beyond uppder bound!");
    }

    auto offset = BlockOffset(row);
    auto block = BlockPosition(row) + offset / bit_width_;
    auto col_offset = offset % bit_width_;

    // calculate the position of one number
    uint64_t bounds = bit_width_ - col_offset;
    uint64_t base_mask = (1ull << bits_) - 1ull;
    uint64_t mask = ~(base_mask << col_offset);

    data_[block] &= mask;
    data_[block] |= (((uint64_t)value & base_mask) << col_offset);

    // in the bounds situation
    if (bounds < bits_) {
      mask = ~(base_mask >> bounds);
      data_[block + 1] &= mask;
      data_[block + 1] |= (((uint64_t)value & base_mask) >> bounds);
    }
  };

  // clear the attribut vector content
  void Clear() {
    size_ = 0;
    free(data_);
    data_ = NULL;
  }

  void Resize(uint64_t row_number) { Resize(row_number, bits_); }

  // this is uesd for delta data, the new_bits should be caclulate outside
  void Resize(uint64_t row_number, int new_bits) {
    if (row_number < rows_) {
      return;
    }

    uint64_t block_numbers = row_number * new_bits / bit_width_ + 1;

    if (block_numbers == blocks_ && new_bits == bits_) {
      // no need to do any thing
      rows_ = row_number;
      return;
    }

    storage_unit_t* data = AllocMemory(block_numbers);

    if (new_bits == bits_) {
      // no need to expand the bit width
      ::memcpy(data, data_, size_);
      Clear();
      data_ = data;
    } else {
      std::vector<T> old_value_set;
      for (size_t i = 0; i < rows_; ++i) {
        T value = Get(i);
        old_value_set.push_back(value);
      }

      // set new data
      bits_ = new_bits;
      storage_unit_t* old_data = data_;
      data_ = data;

      for (size_t i = 0; i < rows_; ++i) {
        Set(i, old_value_set[i]);
      }

      free(old_data);
      old_data = NULL;
    }

    blocks_ = block_numbers;
    size_ = block_numbers * sizeof(storage_unit_t);
    rows_ = row_number;
  }

  size_t Capacity() const { return size_; };

  // get the rows of the bitcompressed vector
  size_t rows() const { return rows_; }

  size_t bits() const { return bits_; }

  size_t blocks() const { return blocks_; }

  // get the used size of the bitcompressed,( unit: bytes )
  size_t size() const { return size_; }

  storage_unit_t* data() const { return data_; }

  // make a deep copy of the base attribute vector
  std::shared_ptr<BaseAttributeVector<T> > Copy() {
    std::shared_ptr<BitCompressedVector> ret =
        std::make_shared<BitCompressedVector>(rows_, bits_);

    ::memcpy(ret->data_, data_, blocks_ * sizeof(storage_unit_t));

    return ret;
  }

  std::shared_ptr<BitCompressedVector<T> > Split(uint64_t begin, uint64_t end) {
    size_t rows = end - begin;

    std::shared_ptr<BitCompressedVector<T> > ret =
        std::make_shared<BitCompressedVector<T> >(rows, bits_);

    for (size_t i = begin; i < end; ++i) {
      T value = Get(i);
      ret->Set((i - begin), value);
    }

    return ret;
  }

  uint64_t MaxValue() {
    T max = Get(0);
    for (int i = 1; i < rows_; ++i) {
      if (Get(i) > max) {
        max = Get(i);
      }
    }

    return max;
  }

  void Print() {
    for (int i = 0; i < rows_; ++i) {
      std::cout << Get(i) << std::endl;
    }
  }

  /* *
   * @describe serialize to string
   * @param[out] result, the result string object
   * */
  void SerializeToString(std::string& result) {
    result.append((char*)&rows_, sizeof(size_t));
    result.append((char*)(&size_), sizeof(size_t));
    result.append((char*)(&blocks_), sizeof(size_t));
    result.append((char*)(&bits_), sizeof(size_t));

    result.append((char*)(data_), size_);
  }

  /* *
   * @describe deserialize from the string
   * @param[in] data, the serialized string
   * */
  void DeserializeFromString(std::string& data) {
    const char* cursor = data.data();

    memcpy(&rows_, cursor, sizeof(size_t));
    cursor += sizeof(size_t);

    memcpy(&size_, cursor, sizeof(size_t));
    cursor += sizeof(size_t);

    memcpy(&blocks_, cursor, sizeof(size_t));
    cursor += sizeof(size_t);

    memcpy(&bits_, cursor, sizeof(size_t));
    cursor += sizeof(size_t);

    if (data_ != NULL) {
      free(data_);
      data_ = NULL;
    }

    // deserialize the data part;
    data_ =
        static_cast<storage_unit_t*>(malloc(blocks_ * sizeof(storage_unit_t)));
    memcpy(data_, cursor, size_);
  }

  // reload operator
 public:
  T operator[](size_t row) { return Get(row); }

  // private interface used in this class
 private:
  void AllocData() {
    blocks_ = (bits_ * rows_) / bit_width_ + 1;
    data_ = AllocMemory(blocks_);
    size_ = sizeof(storage_unit_t) * blocks_;
  }

  inline uint64_t BlockPosition(uint64_t row) const {
    return (bits_ * row) / bit_width_;
  }

  inline uint64_t BlockOffset(uint64_t row) const {
    return (bits_ * row) % bit_width_;
  }

  inline uint64_t BlockNumbers(uint64_t rows) const {
    return ((rows * bits_ + bit_width_ - 1) / bit_width_);
  }

  inline storage_unit_t* AllocMemory(uint64_t block_numbers) {
    auto data = static_cast<storage_unit_t*>(
        malloc(block_numbers * sizeof(storage_unit_t)));

    if (NULL == data) {
      throw std::bad_alloc();
    }
    // set zero
    ::memset(data, 0, block_numbers * sizeof(storage_unit_t));

    return data;
  }

 private:
  // tell the compiler aligned with 16 bytes
  storage_unit_t* data_ __attribute__((aligned(16)));

  // the rows of the vector
  size_t rows_;

  // total size
  size_t size_;

  // total blocks
  size_t blocks_;

  // the bit_width_ of each storage unit
  size_t bits_;

  const static uint64_t bit_width_ = sizeof(storage_unit_t) * 8;
};

#endif
