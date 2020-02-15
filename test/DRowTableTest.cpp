#include "DRowTable.h"
#include <gtest/gtest.h>
#include <vector>
#include "ColumnTable.h"
#include "DGroupKey.h"

#include <memory>

using namespace std;

class DRowTableTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    // meta_data.set_type( StringType );
    meta_data.set_name("TestColumn");
  }

  virtual void TearDown() {}

 protected:
  ColumnTable column_table;
  ColumnMetaData meta_data;
  vector<string> origin_data_1;
  vector<string> origin_data_2;
};

TEST_F(DRowTableTest, TestGenerateAttributeVector) {
  origin_data_1 = {"hotel", "delta",   "frank",   "delta",
                   "apple", "charlie", "charlie", "inbox"};

  std::shared_ptr<DGroupKey<string> > data_list =
      std::make_shared<DGroupKey<string> >();
  data_list->ConstructTreeVector(origin_data_1);
  column_table.AddOneColumn(data_list, meta_data);

  shared_ptr<DGroupKey<string> > groupkey_index =
      dynamic_pointer_cast<DGroupKey<string> >(
          column_table.GetIndexByMetaData(meta_data));

  ASSERT_FALSE((groupkey_index == NULL));

  shared_ptr<vector<uint64_t> > attribute_vector =
      DRowTable::GenerateAttributVector(origin_data_1, groupkey_index->dic());

  vector<uint64_t> expect_attribute_vector = {4, 2, 3, 2, 0, 1, 1, 5};

  for (size_t i = 0; i < expect_attribute_vector.size(); ++i) {
    EXPECT_EQ(expect_attribute_vector[i], (*attribute_vector)[i]);
  }
}

TEST_F(DRowTableTest, TestCreateRowTable) {
  DRowTable row_table(1, "testTable");

  vector<uint64_t> expect_attribute_vector = {4, 2, 3, 2, 0, 1, 1, 5};
  ColumnMetaData meta_data_1, meta_data_2;
  meta_data_1.set_name("column2");
  meta_data_2.set_name("column3");

  row_table.AddRowTableItem(meta_data, expect_attribute_vector, 6);
  row_table.AddRowTableItem(meta_data_1, expect_attribute_vector, 6);
  row_table.AddRowTableItem(meta_data_2, expect_attribute_vector, 6);

  shared_ptr<BitCompressedVector<uint64_t> > column =
      row_table.GetAttributVector(meta_data);

  for (size_t i = 0; i < expect_attribute_vector.size(); ++i) {
    EXPECT_EQ(expect_attribute_vector[i], column->Get(i));
  }
}

TEST_F(DRowTableTest, TestUpdateRowTable) {
  DRowTable row_table(1, "testTable");
  vector<uint64_t> expect_attribute_vector = {4, 2, 3, 2, 0, 1, 1, 5};
  vector<uint64_t> expect_append_vector = {3, 6, 1, 11, 8, 20};

  row_table.AddRowTableItem(meta_data, expect_attribute_vector, 6);
  row_table.AppendRowTableItem(meta_data, expect_append_vector, 21);

  shared_ptr<BitCompressedVector<uint64_t> > column =
      row_table.GetAttributVector(meta_data);

  for (size_t i = 0; i < expect_attribute_vector.size(); ++i) {
    EXPECT_EQ(expect_attribute_vector[i], column->Get(i));
  }

  for (size_t i = expect_attribute_vector.size();
       i < (expect_append_vector.size() + expect_attribute_vector.size());
       ++i) {
    EXPECT_EQ(expect_append_vector[i - expect_attribute_vector.size()],
              column->Get(i));
  }
}

TEST_F(DRowTableTest, TestRefresh) {
  DRowTable row_table(1, "testTable");
  ColumnMetaData column_meta_data;
  column_meta_data.set_name("test_column");

  vector<string> origin_data_1 = {"hotel", "delta",   "frank",   "delta",
                                  "apple", "charlie", "charlie", "inbox"};
  vector<string> origin_data_2 = {"bodo", "bodo", "hotel", "frank"};
  vector<string> origin_data_3 = {"ea", "ba", "aa", "ad"};

  vector<uint64_t> expect_attribute_vector = {4, 2, 3, 2, 0, 1, 1, 5};

  DGroupKey<string> dgroupkey;
  shared_ptr<DGroupKey<string> > update_groupKey =
      make_shared<DGroupKey<string> >();
  shared_ptr<DGroupKey<string> > update_groupKey_1 =
      make_shared<DGroupKey<string> >();

  shared_ptr<vector<uint64_t> > x_vector;
  dgroupkey.ConstructTreeVector(origin_data_1);
  shared_ptr<vector<uint64_t> > attribute =
      DRowTable::GenerateAttributVector(origin_data_1, dgroupkey.dic());

  for (size_t i = 0; i < expect_attribute_vector.size(); ++i) {
    ASSERT_EQ(expect_attribute_vector[i], attribute->at(i));
  }

  update_groupKey->ConstructTreeVector(origin_data_2);
  update_groupKey_1->ConstructTreeVector(origin_data_3);

  update_groupKey_1->Print();

  row_table.AddRowTableItem(column_meta_data, *attribute,
                            dgroupkey.dic()->Size());

  x_vector = dgroupkey.Update(update_groupKey);
  // test if the attribute vector is expected vector

  row_table.Refresh(column_meta_data, *x_vector);

  shared_ptr<BitCompressedVector<uint64_t> > column =
      row_table.GetAttributVector(column_meta_data);

  vector<uint64_t> expected_new_vector = {5, 3, 4, 3, 0, 2, 2, 6};

  for (size_t i = 0; i < expected_new_vector.size(); ++i) {
    EXPECT_EQ(expected_new_vector[i], column->Get(i));
  }

  x_vector = dgroupkey.Update(update_groupKey_1);
  x_vector = dgroupkey.Update(update_groupKey);
  x_vector = dgroupkey.Update(update_groupKey_1);
  x_vector = dgroupkey.Update(update_groupKey_1);
  uint64_t dic_size = dgroupkey.dic()->Size();
  x_vector = dgroupkey.Update(update_groupKey_1);

  EXPECT_EQ(dic_size, x_vector->size());
  for (auto ix : *x_vector) {
    std::cout << ix << std::endl;
  }
}
