/*
 * =================================================================
 *
 *            Filename:    ColumnTable.h
 *
 *         Description:    the table stored in the storage system
 *                      which is cut by column
 *             Version:    v1.0
 *             Created:    2015-09-23 17:27
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */

#ifndef COLUMNTABLE_H_
#define COLUMNTABLE_H_
#include "AbstractIndex.h"
#include "BaseSliceSet.h"
#include "ColumnMetaData.h"
#include "ColumnSliceSet.h"
#include "StorageTypes.h"
#include "TableMetaData.h"

#include <glog/logging.h>
#include <boost/thread.hpp>
#include <boost/thread/lock_factories.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class ColumnTable {
 private:
  // define the column index type such as groupkey index
  typedef std::shared_ptr<AbstractIndex> ColumnIndexType;
  typedef std::shared_ptr<BaseSliceSet> ColumnSlicePtr;
  // bind column meta data and indextype as one type
  typedef std::pair<ColumnMetaData, ColumnSlicePtr> ColumnSlicePair;
  // define he column index list type;
  typedef std::vector<ColumnSlicePair> ColumnIndexListType;

 public:
  ColumnTable() {}
  ColumnTable(int db_id, std::string table_name)
      : meta_data_(db_id, table_name) {}

  ~ColumnTable() {}

  int db_id() const { return meta_data_.db_id(); }

  void set_db_id(int db_id) { meta_data_.set_db_id(db_id); }

  std::string table_name() const { return meta_data_.name(); }

  void set_table_name(std::string table_name) {
    meta_data_.set_name(table_name);
  }

  void SetBeginEntry(ColumnMetaData meta_data, uint64_t slice_no,
                     uint64_t begin_entry) {
    ColumnSlicePtr column_slice_set = GetSliceSetByMetaData(meta_data);

    if (NULL == column_slice_set)
      throw std::invalid_argument("can't find the slice set");

    std::shared_ptr<AbstractIndex> slice_ptr =
        column_slice_set->FindSliceBySliceNo(slice_no);

    if (slice_ptr == NULL)
      throw std::invalid_argument("can't find the column slice");

    slice_ptr->SetDicBeginEntry(begin_entry);
  }

  void AddOneColumn(ColumnIndexType column_index, ColumnMetaData meta_data,
                    uint64_t slice_no) {
    ColumnSlicePtr column_slice_set = GetSliceSetByMetaData(meta_data);

    if (NULL == column_slice_set) {
      column_slice_set = CreateOneColumn(meta_data);
    }

    column_slice_set->AddOneSlice(column_index, slice_no);
  }

  std::shared_ptr<BaseSliceSet> CreateOneColumn(ColumnMetaData &meta_data) {
    std::shared_ptr<BaseSliceSet> slice;
    switch (meta_data.type()) {
      case IntegerType:
        slice = std::make_shared<ColumnSliceSet<int64_t> >();
        break;
      case FloatType:
      case DoubleType:
        slice = std::make_shared<ColumnSliceSet<double> >();
        break;
      case StringType:
        slice = std::make_shared<ColumnSliceSet<std::string> >();
        break;
      default:
        break;
    }

    column_list_.push_back({meta_data, slice});
    return slice;
  }

  std::shared_ptr<std::vector<int64_t> > UpdateOneColumn(
      ColumnIndexType delta_data,
      std::shared_ptr<std::vector<int64_t> > &delta_x_vector,
      ColumnMetaData meta_data, uint64_t slice_no, void *new_dic_lower_bound,
      void *new_dic_upper_bound, uint64_t &new_dic_size,
      uint64_t &dic_size_expan);

  std::shared_ptr<std::vector<int64_t> > UpdateOneColumn(
      ColumnIndexType delta_data,
      std::shared_ptr<std::vector<int64_t> > &delta_x_vector,
      ColumnMetaData meta_data, uint64_t slice_no, uint64_t &new_dic_size,
      uint64_t &dic_size_expan);

  bool IsSlicNoExist(ColumnMetaData &meta_data, uint64_t slice_no) {
    ColumnSlicePtr column_slice_set = GetSliceSetByMetaData(meta_data);

    if (column_slice_set != NULL) {
      return column_slice_set->IsSliceNoExist(slice_no);
    }

    return false;
  }

  std::shared_ptr<BaseSliceSet> GetSliceSetByMetaData(
      ColumnMetaData &meta_data);
  std::shared_ptr<BaseSliceSet> GetSliceSetByColumnName(
      std::string &column_name);

  DataType GetColumnTypeByColumnName(std::string &column_name);

  void Print() {
    meta_data_.Print();
    for (auto ix : column_list_) {
      ix.first.Print();
      ix.second->Print();
    }
  }

  uint64_t StatisticAllColumn();

 public:
  TableMetaData meta_data_;
  ColumnIndexListType column_list_;
};

#endif
