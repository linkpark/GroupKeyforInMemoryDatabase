#include "TableManager.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include <cmath>
#include <iostream>
#include <memory>

using namespace std;

class TableManagerTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    ColumnMetaData column_meta_data("name_1", StringType);
    uint64_t slice_no(0);

    vector<std::string> origin_data = {"hotel", "delta",   "frank",   "delta",
                                       "apple", "charlie", "charlie", "inbox"};

    std::shared_ptr<DGroupKey<std::string> > column_index =
        std::make_shared<DGroupKey<std::string> >();

    column_index->ConstructTreeVector(origin_data);

    std::shared_ptr<ColumnTable> new_table =
        TableManager::GetInstance()->CreateColumnTable(1, "test");

    new_table->AddOneColumn(column_index, column_meta_data, slice_no);
  }

  virtual void TearDown() {}
};

TEST_F(TableManagerTest, testGetAndSet) {
  std::shared_ptr<ColumnTable> new_table =
      TableManager::GetInstance()->CreateColumnTable(1, "test");

  ASSERT_FALSE((NULL == new_table));
}

TEST_F(TableManagerTest, testImportDataAndExtractData) {
  std::shared_ptr<ColumnTable> table =
      TableManager::GetInstance()->GetColumnTable(1, "test");
  uint64_t slice_no(0);

  ColumnMetaData column_meta_data("name_1", StringType);

  EXPECT_FALSE((table == NULL));

  std::shared_ptr<ColumnSliceSet<std::string> > column_slice_set =
      std::dynamic_pointer_cast<ColumnSliceSet<std::string> >(
          table->GetSliceSetByMetaData(column_meta_data));

  std::shared_ptr<DGroupKey<std::string> > column_index =
      std::dynamic_pointer_cast<DGroupKey<std::string> >(
          column_slice_set->FindSliceBySliceNo(slice_no));

  EXPECT_FALSE((column_index == NULL));

  vector<std::string> expect_dic = {"apple", "charlie", "delta",
                                    "frank", "hotel",   "inbox"};

  for (size_t i = 0; i < expect_dic.size(); ++i) {
    EXPECT_EQ(expect_dic[i], column_index->dic()->GetValueForValueId(i));
  }

  TableManager::GetInstance()->StatisticAll();
}
