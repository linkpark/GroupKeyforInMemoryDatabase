/*
 * =================================================================
 *
 *            Filename:    TableManager.h
 *
 *         Description:    the table manager, a singleton class, which is
 *                      used to manage row compressed table and column table
 *
 *             Version:    v1.0
 *             Created:    2015-09-28 17:09
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */

#ifndef TABLEMANAGER_H_
#define TABLEMANAGER_H_
#include <boost/thread.hpp>
#include <boost/thread/lock_factories.hpp>
#include <functional>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>
#include "ColumnTable.h"
#include "DRowTable.h"
#include "Singleton.h"

class TableManager : public Singleton<TableManager> {
  struct TableMetaDataHash {
    size_t operator()(const TableMetaData &meta_data) const {
      std::hash<std::string> hash_func;
      return hash_func(meta_data.name());
    }
  };

  struct TableMetaDataEque {
    bool operator()(const TableMetaData &left,
                    const TableMetaData &right) const {
      return left == right;
    }
  };

  typedef std::unordered_map<TableMetaData, std::shared_ptr<ColumnTable>,
                             TableMetaDataHash, TableMetaDataEque>
      ColumnTableMap;
  typedef std::unordered_map<TableMetaData, std::shared_ptr<DRowTable>,
                             TableMetaDataHash, TableMetaDataEque>
      DRowTableMap;

  friend class Singleton<TableManager>;

 private:
  TableManager() {}

 public:
  virtual ~TableManager() {}

 public:
  std::shared_ptr<ColumnTable> CreateColumnTable(int db_id,
                                                 std::string table_name);

  /* *
   * @describe get the column table
   * @param[in] db_id, database id
   * @param[in] table_name, the table name
   *
   * @return if find return the smart pointer, else return NULL;
   * */
  std::shared_ptr<ColumnTable> GetColumnTable(int db_id,
                                              std::string table_name);

  std::shared_ptr<DRowTable> CreateDRowTable(int db_id, std::string row_table);
  std::shared_ptr<DRowTable> GetDRowTable(int db_id, std::string row_table);

  void StatisticAll();

 private:
  ColumnTableMap column_table_set_;
  DRowTableMap row_table_set_;
  boost::mutex mu_column_table_;
  boost::mutex mu_row_table_;
};

#endif
