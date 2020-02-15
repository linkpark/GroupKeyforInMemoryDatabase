#include "ColumnTable.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include <cmath>
#include <iostream>
#include <memory>

#include <stdio.h>
#include <unistd.h>
#include <string>

using namespace std;

class ColumnTableTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    column_table = make_shared<ColumnTable>();
    meta_data.set_type(StringType);
    meta_data.set_name("TestColumn");
    uint64_t slice_no(0);

    origin_data_1 = {"hotel", "delta",   "frank",   "delta",
                     "apple", "charlie", "charlie", "inbox"};
    origin_data_2 = {"bodo", "bodo", "hotel", "frank"};

    std::shared_ptr<DGroupKey<std::string> > data_list =
        std::make_shared<DGroupKey<std::string> >();
    data_list->ConstructTreeVector(origin_data_1);

    column_table->AddOneColumn(data_list, meta_data, slice_no);
    //column_table->Print();
  }

  virtual void TearDown() {}

 protected:
  shared_ptr<ColumnTable> column_table;
  ColumnMetaData meta_data;
  vector<std::string> origin_data_1;
  vector<std::string> origin_data_2;
};

// test the column slice set
TEST_F(ColumnTableTest, TestGetColumnSliceSet) {
  std::shared_ptr<BaseSliceSet> ret =
      dynamic_pointer_cast<ColumnSliceSet<std::string> >(
          column_table->GetSliceSetByMetaData(meta_data));

  EXPECT_FALSE((NULL == ret));
}

TEST_F(ColumnTableTest, TestColumnData) {
  vector<int64_t> new_origin_data = {1, 1, 1, 1, 4, 4, 6, 7};
  std::shared_ptr<DGroupKey<int64_t> > new_data_list =
      std::make_shared<DGroupKey<int64_t> >();

  new_data_list->ConstructTreeVector(new_origin_data);
  ColumnMetaData new_meta_data;
  new_meta_data.set_name("test_new_column");
  new_meta_data.set_type(IntegerType);
  uint64_t slice_no(3);

  column_table->AddOneColumn(new_data_list, new_meta_data, slice_no);

  std::shared_ptr<ColumnSliceSet<int64_t> > column_slice_set =
      dynamic_pointer_cast<ColumnSliceSet<int64_t> >(
          column_table->GetSliceSetByMetaData(new_meta_data));

  EXPECT_FALSE((NULL == column_slice_set));

  // test if get the same groupkey
  std::shared_ptr<DGroupKey<int64_t> > index =
      dynamic_pointer_cast<DGroupKey<int64_t> >(
          column_slice_set->FindSliceBySliceNo(slice_no));

  for (size_t i = 0; i < new_data_list->dic()->Size(); ++i) {
    EXPECT_EQ(new_data_list->GetDicValueById(i), index->GetDicValueById(i));
  }
}

TEST_F(ColumnTableTest, TestColumnDataUpdateWithLowerBoundChange) {
  vector<std::string> origin_data_3 = {"hotel",   "delta",   "frank", "delta",
                                       "charlie", "charlie", "inbox"};
  vector<std::string> origin_data_4 = {"apple", "bodo", "bodo", "hotel",
                                       "frank"};
  ColumnMetaData meta_data_new;
  meta_data_new.set_type(StringType);
  meta_data_new.set_name("TestColumnTwo");
  uint64_t slice_no(1);

  std::shared_ptr<DGroupKey<std::string> > new_data_list =
      std::make_shared<DGroupKey<std::string> >();
  new_data_list->ConstructTreeVector(origin_data_3);

  column_table->AddOneColumn(new_data_list, meta_data_new, slice_no);

  shared_ptr<DGroupKey<std::string> > delta_data =
      std::make_shared<DGroupKey<std::string> >();

  std::string new_lower_bound;
  std::string new_upper_bound;

  delta_data->ConstructTreeVector(origin_data_4);

  std::string old_lower_bound("charlie");
  std::string old_upper_bound("inbox");

  shared_ptr<vector<int64_t> > delta_x_vector;
  uint64_t new_dic_size;
  uint64_t dic_expan;

  shared_ptr<vector<int64_t> > x_vector =
      column_table->UpdateOneColumn(delta_data, delta_x_vector, meta_data_new,
                                    slice_no, new_dic_size, dic_expan);

  EXPECT_EQ(7, new_dic_size);
  EXPECT_EQ(2, dic_expan);
}

TEST_F(ColumnTableTest, TestColumnDataUpdate) {
  shared_ptr<DGroupKey<std::string> > delta_data =
      std::make_shared<DGroupKey<std::string> >();

  std::string new_lower_bound;
  std::string new_upper_bound;

  delta_data->ConstructTreeVector(origin_data_2);

  std::string old_lower_bound("apple");
  std::string old_upper_bound("inbox");

  shared_ptr<vector<int64_t> > delta_x_vector;
  uint64_t new_dic_size;
  uint64_t dic_size_expan;

  shared_ptr<vector<int64_t> > x_vector = column_table->UpdateOneColumn(
      delta_data, delta_x_vector, meta_data, 0, &new_lower_bound,
      &new_upper_bound, new_dic_size, dic_size_expan);

  vector<int64_t> expect_x_vector = {0, 1, 1, 1, 1, 1};

  for (size_t i = 0; i < expect_x_vector.size(); ++i) {
    EXPECT_EQ(expect_x_vector[i], (*x_vector)[i]);
  }

  EXPECT_EQ(old_lower_bound, new_lower_bound);
  EXPECT_EQ(old_upper_bound, new_upper_bound);
  EXPECT_EQ(7, new_dic_size);
  EXPECT_EQ(1, dic_size_expan);
}
