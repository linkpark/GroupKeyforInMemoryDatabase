/*
 * =================================================================
 *
 *            Filename:    ColumnTable.cpp
 *
 *         Description:    the column table is a column set of the column
 *                      store
 *
 *             Version:    v1.0
 *             Created:    2015-09-23 18:58
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */

#include "ColumnTable.h"
#include <glog/logging.h>
#include <sstream>
#include "DGroupKey.h"

std::shared_ptr<BaseSliceSet> ColumnTable::GetSliceSetByMetaData(
    ColumnMetaData &meta_data) {
  for (auto ix = column_list_.begin(); ix != column_list_.end(); ++ix) {
    if (ix->first.IsMatch(meta_data)) {
      return ix->second;
    }
  }

  return NULL;
}

std::shared_ptr<BaseSliceSet> ColumnTable::GetSliceSetByColumnName(
    std::string &column_name) {
  for (auto ix = column_list_.begin(); ix != column_list_.end(); ++ix) {
    if (ix->first.name() == column_name) {
      return ix->second;
    }
  }

  return NULL;
}

DataType ColumnTable::GetColumnTypeByColumnName(std::string &column_name) {
  for (auto ix = column_list_.begin(); ix != column_list_.end(); ++ix) {
    if (ix->first.name() == column_name) {
      return ix->first.type();
    }
  }

  return UnkownType;
}

// add thread controller, the dic_lower_bound is used for select slice
std::shared_ptr<std::vector<int64_t> > ColumnTable::UpdateOneColumn(
    std::shared_ptr<AbstractIndex> delta_data,
    std::shared_ptr<std::vector<int64_t> > &delta_x_vector,
    ColumnMetaData meta_data, uint64_t slice_no, void *new_dic_lower_bound,
    void *new_dic_upper_bound, uint64_t &new_dic_size,
    uint64_t &dic_size_expan) {
  std::shared_ptr<std::vector<int64_t> > result_index;
  for (size_t i = 0; i < column_list_.size(); ++i) {
    if (column_list_[i].first.IsMatch(meta_data)) {
      result_index = column_list_[i].second->Update(
          delta_data, delta_x_vector, slice_no, new_dic_lower_bound,
          new_dic_upper_bound, new_dic_size, dic_size_expan);
    }
  }

  return result_index;
}
std::shared_ptr<std::vector<int64_t> > ColumnTable::UpdateOneColumn(
    ColumnIndexType delta_data,
    std::shared_ptr<std::vector<int64_t> > &delta_x_vector,
    ColumnMetaData meta_data, uint64_t slice_no, uint64_t &new_dic_size,
    uint64_t &dic_size_expan) {
  std::shared_ptr<std::vector<int64_t> > result_index;
  for (size_t i = 0; i < column_list_.size(); ++i) {
    if (column_list_[i].first.IsMatch(meta_data)) {
      result_index = column_list_[i].second->Update(
          delta_data, delta_x_vector, slice_no, new_dic_size, dic_size_expan);
    }
  }

  return result_index;
}

uint64_t ColumnTable::StatisticAllColumn() {
  uint64_t sum = 0;
  for (auto it = column_list_.begin(); it != column_list_.end(); ++it) {
    std::stringstream str_stream;
    str_stream << std::endl
               << "----column name:" << it->first.name() << std::endl;
    str_stream << "----column rows:" << it->second->Rows() << std::endl;
    LOG(INFO) << str_stream.str();

    sum += it->second->Statistic();
  }

  LOG(INFO) << "total column number: " << column_list_.size();
  return sum;
}
