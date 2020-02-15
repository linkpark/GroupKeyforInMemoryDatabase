/*
 * =================================================================
 *
 *            Filename:    TableMetaData.h
 *
 *         Description:  the table meta data structure
 *
 *             Version:    v1.0
 *             Created:    2015-10-08 15:33
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */
#ifndef TABLEMETADATA_H_
#define TABLEMETADATA_H_
#include <iostream>
#include <string>

class TableMetaData {
 public:
  TableMetaData() : db_id_(-1), name_("") {}

  TableMetaData(int db_id, std::string name) : db_id_(db_id), name_(name) {}

  ~TableMetaData() {}

  void set_db_id(int db_id) { db_id_ = db_id; }

  void set_name(std::string name) { name_ = name; }

  int db_id() const { return db_id_; }

  std::string name() const { return name_; }

  bool IsMatch(const TableMetaData& table_meta_data) {
    if (table_meta_data.db_id_ == db_id_ && table_meta_data.name_ == name_) {
      return true;
    }

    return false;
  }

  friend bool operator==(const TableMetaData& left,
                         const TableMetaData& right) {
    if (left.db_id_ == right.db_id_ && left.name_ == right.name_) {
      return true;
    }

    return false;
  }

  void Print() {
    std::cout << "db id:" << db_id_ << std::endl;
    std::cout << "table name:" << name_ << std::endl;
  }

 private:
  int db_id_;
  std::string name_;
};

#endif
