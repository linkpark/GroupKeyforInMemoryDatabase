#include "ToolSet.h"
#include <gtest/gtest.h>
#include <string>

using namespace std;

class ToolSetTest : public ::testing::Test {
 protected:
  void SetUp(void) { origin_data_1 = {1, 2, 3, 4, 5, 5, 5, 6, 7, 8, 9}; }

  void TearDown(void) {}

 protected:
  vector<int> origin_data_1;
};

TEST_F(ToolSetTest, TestSerializeAndDeserialize) {
  string result;
  ToolSet::VectorSerializeToString<int>(origin_data_1, result);

 // cout << "serialize size:" << result.capacity() << endl;

  std::shared_ptr<vector<int> > target_data;
  ToolSet::VectorDeserializeFromString(target_data, result);

  for (size_t i = 0; i < origin_data_1.size(); ++i) {
    EXPECT_EQ(origin_data_1[i], target_data->at(i));
  }
}
