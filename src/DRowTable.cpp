/*
 * =================================================================
 *
 *            Filename:    DRowTable.cpp
 *
 *         Description:    the implements of the DRowTable
 *
 *             Version:    v1.0
 *             Created:    2015-09-28 11:46
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */
#include "DRowTable.h"

void DRowTable::AddRowTableItem(ColumnMetaData meta_data,
                                std::vector<uint64_t> &attribute_vector,
                                size_t dic_size) {
  int bits = (int)log2(dic_size) + 1;
  std::shared_ptr<BitCompressedVector<uint64_t> > attribute =
      std::make_shared<BitCompressedVector<uint64_t> >(attribute_vector.size(),
                                                       bits);

  for (size_t i = 0; i < attribute_vector.size(); ++i) {
    attribute->Set(i, attribute_vector[i]);
  }

  // add the new pair to the RowTablePair
  column_meta_data_list_.push_back(meta_data);
  row_table_.push_back(attribute);
}

void DRowTable::AppendRowTableItem(
    ColumnMetaData meta_data, std::vector<uint64_t> &delta_attribute_vector,
    size_t dic_size) {
  std::shared_ptr<BitCompressedVector<uint64_t> > attribute_vector =
      GetAttributVector(meta_data);

  if (attribute_vector == NULL)
    throw std::invalid_argument("can't find the column");

  uint64_t old_rows = attribute_vector->rows();
  uint64_t new_rows = attribute_vector->rows() + delta_attribute_vector.size();

  int bits = (int)log2(dic_size) + 1;

  attribute_vector->Resize(new_rows, bits);

  for (size_t i = old_rows; i < new_rows; ++i)
    attribute_vector->Set(i, delta_attribute_vector[i - old_rows]);
}

void DRowTable::Refresh(ColumnMetaData &meta_data,
                        std::vector<uint64_t> &x_vector) {
  std::shared_ptr<BitCompressedVector<uint64_t> > attribute_vector =
      GetAttributVector(meta_data);

  if (attribute_vector == NULL) {
    LOG(ERROR) << "no matched column";
    throw std::invalid_argument("no matched column");
  }

  uint64_t tmp;

  for (size_t i = 0; i < attribute_vector->rows(); ++i) {
    tmp = attribute_vector->Get(i);
    if (tmp > x_vector.size()) {
      LOG(ERROR) << "x_vector size is not matched!"
                 << "column name:" << meta_data.name()
                 << "x_vector size:" << x_vector.size()
                 << "old dic size:" << tmp;
      throw std::invalid_argument("x_vector size is not matched");
    }

    tmp = tmp + x_vector[tmp];
    attribute_vector->Set(i, tmp);
  }
}

DRowTable::VectorPtr DRowTable::GetValue(
    std::string &column_name, const std::vector<uint64_t> &row_id_set) {
  VectorPtr ret = std::make_shared<std::vector<uint64_t> >();

  for (size_t i = 0; i < column_meta_data_list_.size(); ++i) {
    if (column_meta_data_list_[i].name() == column_name) {
      for (size_t j = 0; j < row_id_set.size(); ++j) {
        ret->push_back(row_table_[i]->Get(row_id_set[j]));
      }
    }
  }

  return ret;
}

DRowTable::VectorPtr DRowTable::GetFullValue(std::string &column_name) {
  VectorPtr ret = std::make_shared<std::vector<uint64_t> >();

  for (size_t i = 0; i < column_meta_data_list_.size(); ++i) {
    if (column_meta_data_list_[i].name() == column_name) {
      for (size_t j = 0; j < row_table_[i]->rows(); ++j) {
        ret->push_back(row_table_[i]->Get(j));
      }
    }
  }

  return ret;
}

void DRowTable::Print() {
  std::cout << "table name:" << meta_data_.name() << std::endl;
  for (size_t i = 0; i < column_meta_data_list_.size(); ++i) {
    std::cout << "--column name:" << column_meta_data_list_[i].name()
              << std::endl;
    std::cout << "--";
    for (size_t j = 0; j < row_table_[i]->rows(); ++j) {
      std::cout << row_table_[i]->Get(j) << " ";
    }
    std::cout << std::endl;
  }
}
