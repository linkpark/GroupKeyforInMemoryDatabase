/*
 * =================================================================
 *
 *            Filename:    AbstractIndex.h
 *
 *         Description:    define the interface about the abstract index
 *                      this class is an interface
 *             Version:    v1.0
 *             Created:    2015-09-23 16:55
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */

#ifndef ABSTRACTINDEX_H_
#define ABSTRACTINDEX_H_
#include <memory>
#include <vector>
#include "StorageTypes.h"

class AbstractIndex : public std::enable_shared_from_this<AbstractIndex> {
 public:
  AbstractIndex() {}
  virtual ~AbstractIndex() {}

 public:
  virtual void WriteLock() = 0;

  virtual void UnLock() = 0;

  virtual void Print() = 0;

  // unit bytes;
  virtual uint64_t Rows() = 0;
  virtual uint64_t DictRows() = 0;

  virtual std::shared_ptr<std::vector<int64_t> > Update(
      std::shared_ptr<AbstractIndex> deltaData,
      std::shared_ptr<std::vector<int64_t> > &delta_x_vector) = 0;

  virtual uint64_t Statistic() = 0;

  // the serialize interface
  virtual std::shared_ptr<AbstractIndex> SplitByDicEntryList(
      std::vector<uint64_t> &dict_entry_list) = 0;
  virtual std::shared_ptr<AbstractIndex> SplitByRowIdList(
      std::vector<uint64_t> &row_id_list,
      std::vector<uint64_t> &dict_id_list) = 0;

  virtual void SerializeToString(std::string &result) = 0;
  virtual void DeserializeFromString(std::string &data) = 0;
  virtual void SetDicBeginEntry(uint64_t begin_entry) = 0;
  virtual DataType GetColumnType() = 0;
  virtual uint64_t GetDicBeginEntry() const = 0;

  std::shared_ptr<AbstractIndex> get_shared_ptr() { return shared_from_this(); }
};

#endif
