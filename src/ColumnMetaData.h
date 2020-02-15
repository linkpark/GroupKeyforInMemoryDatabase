/*
 * =================================================================
 *
 *            Filename:    ColumnMetaData.h
 *
 *         Description:    descript the column metadata, mainly include
 *                  column name and column data type
 *
 *             Version:    v1.0
 *             Created:    2015-09-23 17:07
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */

#ifndef COLUMNMETADATA_H_
#define COLUMNMETADATA_H_
#include <stdint.h>
#include <iostream>
#include "StorageTypes.h"

class ColumnMetaData {
 public:
  explicit ColumnMetaData(std::string name, DataType type)
      : type_(type), name_(name) {}

  ColumnMetaData() : type_(UnkownType), name_(""), bit_width_(0) {}

  ~ColumnMetaData() {}

 public:
  void set_type(DataType type) { type_ = type; }

  DataType type() const { return type_; }

  void set_name(const char* name) { name_ = name; }

  void set_name(std::string name) { name_ = name; }

  std::string name() const { return name_; }

  void set_bit_width(uint64_t bit_width) { bit_width_ = bit_width; }

  uint64_t bit_width() const { return bit_width_; }

  // right now we differ from column by column name,
  // maybe in the future we differ column by both column name and
  // column type;
  bool IsMatch(const ColumnMetaData& meta_data) {
    if (meta_data.name_ == name_) return true;

    return false;
  }

  // in one table, the column name will be unique
  bool IsMatch(const std::string colum_name) {
    if (colum_name == name_) return true;

    return false;
  }

  // reload the '==' operator
  friend bool operator==(const ColumnMetaData& left,
                         const ColumnMetaData& right) {
    if (left.name_ == right.name_) {
      return true;
    }

    return false;
  }

  void Print() { std::cout << name_ << std::endl; }

 private:
  DataType type_;
  std::string name_;
  uint64_t bit_width_;
};

#endif
