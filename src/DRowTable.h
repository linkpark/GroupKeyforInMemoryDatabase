/*
 * =================================================================
 *
 *            Filename:    DRowTable.h
 *
 *         Description:    the row table of groupkey index
 *
 *             Version:    v1.0
 *             Created:    2015-09-28 11:20
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */

#ifndef DROWTABLE_H_
#define DROWTABLE_H_
#include "BitCompressedVector.h"
#include "ColumnMetaData.h"
#include "OrderedDictionary.h"
#include "TableMetaData.h"

class DRowTable {
 protected:
  typedef std::pair<ColumnMetaData,
                    std::shared_ptr<BitCompressedVector<uint64_t> > >
      RowTablePair;

  typedef std::shared_ptr<std::vector<uint64_t> > VectorPtr;

 public:
  explicit DRowTable(int db_id, std::string table_name)
      : meta_data_(db_id, table_name) {}

  virtual ~DRowTable() {}

  void AddRowTableItem(ColumnMetaData meta_data,
                       std::vector<uint64_t> &attribute_vector,
                       size_t dic_size);

  void Refresh(ColumnMetaData &meta_data, std::vector<uint64_t> &x_vector);

  /* *
   * @describe append the attribute vector to the data
   * @param[in] meta_data
   * @param[in] attribute_vector
   * @param[in] dic_size the new dictionary size
   * */
  void AppendRowTableItem(ColumnMetaData meta_data,
                          std::vector<uint64_t> &attribute_vector,
                          size_t dic_size);

  VectorPtr GetValue(std::string &column_name,
                     const std::vector<uint64_t> &row_id_set);

  VectorPtr GetFullValue(std::string &column_name);

  std::shared_ptr<BitCompressedVector<uint64_t> > GetAttributVector(
      ColumnMetaData &meta_data) {
    for (size_t i = 0; i < column_meta_data_list_.size(); ++i) {
      if (column_meta_data_list_[i].IsMatch(meta_data)) {
        return row_table_[i];
      }
    }

    return NULL;
  }

  /* *
   * @desc generate attribute vector
   * @param[in] origin_data, the origin_data of the column
   * @param[in] dic, the dic of the groupkey
   *
   * @return bit compressed vector about the row table
   * */
  template <class T>
  static std::shared_ptr<std::vector<uint64_t> > GenerateAttributVector(
      std::vector<T> origin_data, std::shared_ptr<OrderedDictionary<T> > dic) {
    std::shared_ptr<std::vector<uint64_t> > ret =
        std::make_shared<std::vector<uint64_t> >();

    for (auto i = origin_data.begin(); i != origin_data.end(); ++i) {
      uint64_t value_id = dic->GetValueIdForValue(*i);

      ret->push_back(value_id);
    }

    return ret;
  }

  // used for debug, to print the object;
  void Print();
  uint64_t Statistic() {
    uint64_t sum = 0;
    for (auto ix : row_table_) {
      sum += ix->size();
    }

    return sum;
  }

 private:
  TableMetaData meta_data_;
  std::vector<ColumnMetaData> column_meta_data_list_;
  std::vector<std::shared_ptr<BitCompressedVector<uint64_t> > > row_table_;
};

#endif
