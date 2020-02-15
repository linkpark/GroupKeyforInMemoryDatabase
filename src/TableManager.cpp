/*
 * =================================================================
 *
 *            Filename:    TableManager.cpp
 *
 *         Description:    the implements of TableManager class
 *
 *             Version:    v1.0
 *             Created:    2015-09-29 10:03
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */
#include "TableManager.h"

// lock the write operation about the create column Table
std::shared_ptr<ColumnTable> TableManager::CreateColumnTable(
    int db_id, std::string table_name) {
  auto lock = boost::make_unique_lock(mu_column_table_);
  TableMetaData meta_data(db_id, table_name);

  column_table_set_.insert(TableManager::ColumnTableMap::value_type(
      meta_data, std::make_shared<ColumnTable>(1, table_name)));

  return column_table_set_.find(meta_data)->second;
}

// lock the read operation about get column table
std::shared_ptr<ColumnTable> TableManager::GetColumnTable(
    int db_id, std::string table_name) {
  auto lock = boost::make_unique_lock(mu_column_table_);
  TableMetaData meta_data(db_id, table_name);

  auto it = column_table_set_.find(meta_data);
  if (it == column_table_set_.end()) {
    return NULL;
  } else {
    return it->second;
  }
}

std::shared_ptr<DRowTable> TableManager::CreateDRowTable(
    int db_id, std::string row_table) {
  // keep the thread safe at mutiple thread enviroment
  // lock when construct, unlock when deconstruct
  auto lock = boost::make_unique_lock(mu_row_table_);
  TableMetaData meta_data(db_id, row_table);

  row_table_set_.insert(TableManager::DRowTableMap::value_type(
      meta_data, std::make_shared<DRowTable>(1, row_table)));

  return row_table_set_.find(meta_data)->second;
}

std::shared_ptr<DRowTable> TableManager::GetDRowTable(int db_id,
                                                      std::string row_table) {
  // keep the thread safe at mutiple thread enviroment
  auto lock = boost::make_unique_lock(mu_row_table_);
  TableMetaData meta_data(db_id, row_table);

  auto it = row_table_set_.find(meta_data);
  if (it == row_table_set_.end()) {
    return NULL;
  } else {
    return it->second;
  }
}

// doing the statistic operation
void TableManager::StatisticAll() {
  uint64_t sum = 0;
  uint64_t row_table_sum = 0;
  if (column_table_set_.size() == 0) LOG(INFO) << "table list is empty";

  for (auto ix = column_table_set_.begin(); ix != column_table_set_.end();
       ++ix) {
    LOG(INFO) << "table name: " << ix->first.name() << std::endl;
    sum += ix->second->StatisticAllColumn();
  }

  for (auto ix = row_table_set_.begin(); ix != row_table_set_.end(); ++ix) {
    row_table_sum += ix->second->Statistic();
  }

  LOG(INFO) << "all column table cost: " << sum << "bytes memory";
  LOG(INFO) << "all rtable cost: " << row_table_sum << "bytes memory";
}
