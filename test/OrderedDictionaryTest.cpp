#include "OrderedDictionary.h"

#include <gtest/gtest.h>
#include <string>

using namespace std;

class OrderedDictionaryTest : public ::testing::Test {
 protected:
  void SetUp(void) {}

  void TearDown(void) {}

 protected:
  vector<string> origin_data = {"a", "b", "c", "d", "e", "f", "h", "i"};
};

TEST_F(OrderedDictionaryTest, TestInterface) {
  OrderedDictionary<string> my_dic;

  for (auto i : origin_data) {
    my_dic.AddValue(i);
  }

  EXPECT_EQ(3, my_dic.FindValueIdForValue("d"));
  ASSERT_ANY_THROW(my_dic.FindValueIdForValue("x"));

  EXPECT_EQ("a", my_dic.GetValueForValueId(0));
  EXPECT_EQ("i", my_dic.GetGreatestValue());
}

TEST_F(OrderedDictionaryTest, TestCopy) {
  OrderedDictionary<string> my_dic;

  for (auto i : origin_data) {
    my_dic.AddValue(i);
  }

  shared_ptr<OrderedDictionary<string> > copy_dic =
      std::dynamic_pointer_cast<OrderedDictionary<string> >(my_dic.Copy());

  EXPECT_TRUE(&my_dic != copy_dic.get());
}

// test the upper bound and lower bound
TEST_F(OrderedDictionaryTest, TestUpperBoundAndLowerBound) {
  OrderedDictionary<string> my_dic;

  for (auto i : origin_data) {
    my_dic.AddValue(i);
  }

  size_t expect_lower_bound = 2;

  EXPECT_EQ(expect_lower_bound, my_dic.FindLowerBoundForValue("c"));
  EXPECT_EQ(6, my_dic.FindUpperBoundForValue("g"));
  EXPECT_EQ(8, my_dic.FindUpperBoundForValue("i"));
  EXPECT_EQ(6, my_dic.FindLowerBoundForValue("g"));
  EXPECT_EQ(0, my_dic.FindLowerBoundForValue("a"));
  EXPECT_EQ(0, my_dic.FindLowerBoundForValue("0"));
}

// test the serialize interface and deserialize interface
TEST_F(OrderedDictionaryTest, TestSerializeAndDeserialize) {
  OrderedDictionary<string> my_dic, my_dic_copy;
  string seriliaze_data;

  for (auto i : origin_data) {
    my_dic.AddValue(i);
  }

  my_dic.SerializeToString(seriliaze_data);

  //std::cout << "the serialize size is " << seriliaze_data.capacity() << "bytes "
  //          << endl;

  my_dic_copy.DeserializeFromString(seriliaze_data);

  ASSERT_EQ(my_dic.Size(), my_dic_copy.Size());
  for (size_t i = 0; i < my_dic.Size(); ++i) {
    ASSERT_EQ(my_dic[i], my_dic_copy[i]);
  }
}

TEST_F(OrderedDictionaryTest, TestSerializeAndDeserializeForString) {
  std::vector<std::string> dstring_origin_data = {"a",    "b",     "dd",   "ee",
                                                  "ffff", "hhhhh", "ifadr"};
  OrderedDictionary<std::string> my_dic, my_dic_copy;
  string seriliaze_data;

  for (auto i : dstring_origin_data) {
    my_dic.AddValue(i);
  }

  my_dic.SerializeToString(seriliaze_data);
  //std::cout << "the serialize size is " << seriliaze_data.capacity() << "bytes "
  //          << endl;

  my_dic_copy.DeserializeFromString(seriliaze_data);
  ASSERT_EQ(my_dic.Size(), my_dic_copy.Size());
  ASSERT_EQ(my_dic.entry_begin(), my_dic_copy.entry_begin());

  for (size_t i = 0; i < my_dic.Size(); ++i) {
    ASSERT_EQ(my_dic[i], my_dic_copy[i]);
  }
}

TEST_F(OrderedDictionaryTest, TestSplit) {
  OrderedDictionary<string> my_dic;

  for (auto i : origin_data) {
    my_dic.AddValue(i);
  }

  std::shared_ptr<OrderedDictionary<string> > result = my_dic.Split(0, 3);

  for (size_t i = 0; i < result->Size(); ++i) {
    ASSERT_EQ((*result)[i], my_dic[i]);
  }
}
