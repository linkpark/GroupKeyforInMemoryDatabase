#include "DGroupKey.h"
#include <gtest/gtest.h>
#include <string>

using namespace std;

class DGroupKeyTest : public ::testing::Test {
 protected:
  void SetUp(void) {
    origin_data_1 = {"hotel", "delta",   "frank",   "delta",
                     "apple", "charlie", "charlie", "inbox"};
    origin_data_2 = {"bodo", "bodo", "hotel", "frank"};
    origin_data_3 = {"hotel",   "delta",   "frank", "delta", "apple",
                     "charlie", "charlie", "inbox", "inbox"};
    origin_data_4 = {2023, 2024, 2078, 2000,  1999, 1987,
                     1807, 2076, 1920, 12.13, 1111, 23232};
    origin_data_5 = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
                     13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
    origin_data_6 = {'s', 'a', 'f', 'x', 'z', 'b', 'h'};
    
  }

  void TearDown(void) {}

 protected:
  vector<string> origin_data_1;
  vector<string> origin_data_2;
  vector<string> origin_data_3;
  vector<double> origin_data_4;
  vector<double> origin_data_5;
  vector<char> origin_data_6;
};

TEST_F(DGroupKeyTest, OriginData1Test) {
  DGroupKey<string> dgroupkey;

  dgroupkey.ConstructTreeVector(origin_data_1);

  shared_ptr<OrderedDictionary<string>> dic = dgroupkey.dic();
  shared_ptr<BitCompressedVector<uint64_t>> index = dgroupkey.index();
  shared_ptr<BitCompressedVector<uint64_t>> position = dgroupkey.position();

  vector<string> expect_dic = {"apple", "charlie", "delta",
                               "frank", "hotel",   "inbox"};
  vector<uint64_t> expect_index = {0, 1, 3, 5, 6, 7, 8};
  vector<uint64_t> expect_position = {4, 5, 6, 1, 3, 2, 0, 7};
  DGroupKey<string> dgroupkey_2;

  for (size_t i = 0; i < expect_dic.size(); ++i) {
    EXPECT_EQ(expect_dic[i], dic->GetValueForValueId(i));
  }

  for (size_t i = 0; i < expect_index.size(); ++i) {
    EXPECT_EQ(expect_index[i], index->Get(i));
  }

  for (size_t i = 0; i < expect_position.size(); ++i) {
    EXPECT_EQ(expect_position[i], position->Get(i));
  }

  vector<uint64_t> id_list = {0, 1, 2};
  shared_ptr<vector<string>> result = dgroupkey.GetDicValueByIdList(id_list);

  for (size_t i = 0; i < 3; ++i) {
    EXPECT_EQ(expect_dic[i], (*result)[i]);
  }

  EXPECT_EQ(expect_dic[0], dgroupkey.GetDicValueById(0));

  vector<string> value_list = {"apple", "delta"};
  shared_ptr<vector<uint64_t>> reslut_position =
      dgroupkey.GetRowIdListByValueList(value_list);
  shared_ptr<vector<uint64_t>> reslut_position_1 =
      dgroupkey.GetRowIdListByDicId(0);
  shared_ptr<vector<uint64_t>> reslut_position_2 =
      dgroupkey.GetRowIdListByDicId(1);

  vector<uint64_t> expect_id_list = {4, 1, 3};
  vector<uint64_t> expect_id_list_1 = {4};
  vector<uint64_t> expect_id_list_2 = {5, 6};

  EXPECT_EQ(expect_id_list.size(), reslut_position->size());
  EXPECT_EQ(expect_id_list_1.size(), reslut_position_1->size());
  EXPECT_EQ(expect_id_list_2.size(), reslut_position_2->size());

  for (size_t i = 0; i < expect_id_list.size(); ++i) {
    EXPECT_EQ(expect_id_list[i], (*reslut_position)[i]);
  }

  for (size_t i = 0; i < expect_id_list_1.size(); ++i) {
    EXPECT_EQ(expect_id_list_1[i], (*reslut_position_1)[i]);
  }

  for (size_t i = 0; i < expect_id_list_2.size(); ++i) {
    EXPECT_EQ(expect_id_list_2[i], (*reslut_position_2)[i]);
  }
}

TEST_F(DGroupKeyTest, TestConstructFunction) {
  vector<string> expect_dic = {"apple", "charlie", "delta",
                               "frank", "hotel",   "inbox"};

  vector<uint64_t> expect_index = {0, 1, 3, 5, 6, 7, 8};
  vector<uint64_t> expect_position = {4, 5, 6, 1, 3, 2, 0, 7};

  shared_ptr<OrderedDictionary<string>> dic =
      make_shared<OrderedDictionary<string>>();

  for (size_t i = 0; i < expect_dic.size(); ++i) {
    dic->AddValue(expect_dic[i]);
  }

  int bits = (int)log2(expect_index[expect_index.size() - 1]) + 1;

  shared_ptr<BitCompressedVector<uint64_t>> index =
      make_shared<BitCompressedVector<uint64_t>>(expect_index.size(), bits);

  for (size_t i = 0; i < expect_index.size(); ++i) {
    index->Set(i, expect_index[i]);
  }

  shared_ptr<BitCompressedVector<uint64_t>> position =
      make_shared<BitCompressedVector<uint64_t>>(expect_position.size(), bits);

  for (size_t i = 0; i < expect_position.size(); ++i) {
    position->Set(i, expect_position[i]);
  }

  shared_ptr<DGroupKey<string>> groupkey_index =
      make_shared<DGroupKey<string>>(dic, index, position);

  for (size_t i = 0; i < expect_dic.size(); ++i) {
    EXPECT_EQ(expect_dic[i], groupkey_index->dic()->GetValueForValueId(i));
  }

  for (size_t i = 0; i < expect_index.size(); ++i) {
    EXPECT_EQ(expect_index[i], groupkey_index->index()->Get(i));
  }

  for (size_t i = 0; i < expect_position.size(); ++i) {
    EXPECT_EQ(expect_position[i], groupkey_index->position()->Get(i));
  }
}

TEST_F(DGroupKeyTest, TestUpdateInterface) {
  DGroupKey<string> dgroupkey;
  shared_ptr<DGroupKey<string>> update_groupKey =
      make_shared<DGroupKey<string>>();
  shared_ptr<vector<int64_t>> x_vector;
  vector<int64_t> expect_x_vector = {0, 1, 1, 1, 1, 1};
  vector<int64_t> expect_delta_x_vector = {1, 3, 3};
  vector<string> expect_merge_dic = {"apple", "bodo",  "charlie", "delta",
                                     "frank", "hotel", "inbox"};

  vector<string> origin_data_ch = {
      "成员", "管理员", "成员",   "群主", "成员",   "成员", "成员",
      "成员", "成员",   "成员",   "成员", "成员",   "成员", "成员",
      "成员", "管理员", "成员",   "成员", "成员",   "成员", "成员",
      "成员", "成员",   "管理员", "成员", "管理员", "成员"};
  vector<uint64_t> expect_merge_index = {0, 1, 3, 5, 7, 9, 11, 12};
  vector<uint64_t> expect_merge_position = {4, 8, 9,  5, 6,  1,
                                            3, 2, 11, 0, 10, 7};

  DGroupKey<string> dgroupkey_1;
  dgroupkey_1.dic()->set_entry_begin(0);
  dgroupkey_1.ConstructTreeVector(origin_data_ch);
  // dgroupkey_1.dic()->Print();

  /*for (size_t i = 0; i < dgroupkey_1.dic()->Size(); ++i) {
    cout << dgroupkey_1.dic()->GetValueForValueId(i) << endl;
  }*/

  dgroupkey.ConstructTreeVector(origin_data_1);
  // dgroupkey.Print();

  shared_ptr<vector<int64_t>> delta_x_vector;
  update_groupKey->ConstructTreeVector(origin_data_2, origin_data_1.size());

  x_vector = dgroupkey.Update(update_groupKey, delta_x_vector);

  shared_ptr<OrderedDictionary<string>> dic = dgroupkey.dic();
  shared_ptr<BitCompressedVector<uint64_t>> index = dgroupkey.index();
  shared_ptr<BitCompressedVector<uint64_t>> position = dgroupkey.position();
  // test the x_vector, which is returned by DGroupKey::Update
  for (size_t i = 0; i < expect_x_vector.size(); ++i) {
    EXPECT_EQ(expect_x_vector[i], (*x_vector)[i]);
  }

  // test the delta_x_vector, which is set by DGroupKey::Update
  EXPECT_EQ(expect_delta_x_vector.size(), (*delta_x_vector).size());
  for (size_t i = 0; i < expect_delta_x_vector.size(); ++i) {
    EXPECT_EQ(expect_delta_x_vector[i], (*delta_x_vector)[i]);
  }

  // test the update dic
  dic->set_entry_begin(1);
  for (size_t i = 1; i < expect_merge_dic.size(); ++i) {
    try {
      EXPECT_EQ(expect_merge_dic[i], dic->GetValueForValueId((i + 1)));
    } catch (...) {
      cerr << "invalid id " << i << "dic size" << dic->Size() << endl;
    }
  }

  // test the update index
  for (size_t i = 0; i < expect_merge_index.size(); ++i) {
    EXPECT_EQ(expect_merge_index[i], (*index)[i]);
  }

  // test the update postion
  for (size_t i = 0; i < expect_merge_index.size(); ++i) {
    EXPECT_EQ(expect_merge_position[i], (*position)[i]);
  }
}

// test the GT and GE operator interface
TEST_F(DGroupKeyTest, TestGTAndGEOperator) {
  DGroupKey<string> dgroupkey;
  dgroupkey.ConstructTreeVector(origin_data_3);
  vector<uint64_t> expect_result_1 = {5, 6, 1, 3, 2, 0, 7, 8};
  vector<string> expect_dic_1 = {"charlie", "delta", "frank", "hotel", "inbox"};
  vector<uint64_t> expect_index_1 = {0, 2, 4, 5, 6, 8};

  vector<uint64_t> expect_result_2 = {4, 5, 6, 1, 3, 2, 0, 7};
  vector<string> expect_dic_2 = {"apple", "charlie", "delta",
                                 "frank", "hotel",   "inbox"};
  vector<uint64_t> expect_index_2 = {0, 1, 3, 5, 6, 7, 9};

  vector<uint64_t> expect_result_3 = {1, 3, 2, 0, 7, 8};
  vector<string> expect_dic_3 = {"delta", "frank", "hotel", "inbox"};
  vector<uint64_t> expect_index_3 = {0, 2, 3, 4, 6};

  shared_ptr<vector<uint64_t>> result_1 =
      dgroupkey.GetGTRowIdListByValue("apple");

  shared_ptr<DGroupKey<string>> result_groupkey_1 =
      dgroupkey.GetGTGroupkeyByValue("apple");

  shared_ptr<vector<uint64_t>> result_2 =
      dgroupkey.GetGERowIdListByValue("apple");

  shared_ptr<DGroupKey<string>> result_groupkey_2 =
      dgroupkey.GetGEGroupkeyByValue("apple");

  shared_ptr<vector<uint64_t>> result_3 =
      dgroupkey.GetGERowIdListByValue("charlie");

  shared_ptr<DGroupKey<string>> result_groupkey_3 =
      dgroupkey.GetGEGroupkeyByValue("charlie");

  shared_ptr<vector<uint64_t>> result_4 = dgroupkey.GetGTRowIdListByValue("cz");

  shared_ptr<DGroupKey<string>> result_groupkey_4 =
      dgroupkey.GetGTGroupkeyByValue("cz");

  // for the test 1;
  // test if the dic is correct
  for (size_t i = 0; i < expect_dic_1.size(); ++i) {
    EXPECT_EQ(expect_dic_1[i],
              (*result_groupkey_1
                    ->dic())[(i + result_groupkey_1->GetDicBeginEntry())]);
  }

  // test if the index is correct
  for (size_t i = 0; i < expect_index_1.size(); ++i) {
    EXPECT_EQ(expect_index_1[i], (*result_groupkey_1->index())[i]);
  }

  // test if the position vector is correct
  for (size_t i = 0; i < expect_result_1.size(); ++i) {
    EXPECT_EQ(expect_result_1[i], (*result_groupkey_1->position())[i]);
    EXPECT_EQ(expect_result_1[i], result_1->at(i));
  }

  // for the test 2
  // test if the dic is correct
  for (size_t i = 0; i < expect_dic_2.size(); ++i) {
    EXPECT_EQ(
        expect_dic_2[i],
        (*result_groupkey_2->dic())[i + result_groupkey_2->GetDicBeginEntry()]);
  }

  // test if the index is correct
  for (size_t i = 0; i < expect_index_2.size(); ++i) {
    EXPECT_EQ(expect_index_2[i], (*result_groupkey_2->index())[i]);
  }

  // test if the position is correct
  for (size_t i = 0; i < expect_result_2.size(); ++i) {
    EXPECT_EQ(expect_result_2[i], result_2->at(i));
    EXPECT_EQ(expect_result_2[i], (*result_groupkey_2->position())[i]);
  }
  // test 3
  // test if the dic is correct
  for (size_t i = 0; i < expect_dic_1.size(); ++i) {
    EXPECT_EQ(
        expect_dic_1[i],
        (*result_groupkey_3->dic())[i + result_groupkey_3->GetDicBeginEntry()]);
  }

  // test if the index is correct
  for (size_t i = 0; i < expect_index_1.size(); ++i) {
    EXPECT_EQ(expect_index_1[i], (*result_groupkey_3->index())[i]);
  }

  // test if position is correct
  for (size_t i = 0; i < expect_result_1.size(); ++i) {
    EXPECT_EQ(expect_result_1[i], (*result_groupkey_3->position())[i]);
    EXPECT_EQ(expect_result_1[i], result_3->at(i));
  }

  // test 4
  // test if the dic is correct
  for (size_t i = 0; i < expect_dic_3.size(); ++i) {
    EXPECT_EQ(
        expect_dic_3[i],
        (*result_groupkey_4->dic())[i + result_groupkey_4->GetDicBeginEntry()]);
  }

  // test if the index is correct
  for (size_t i = 0; i < expect_index_3.size(); ++i) {
    EXPECT_EQ(expect_index_3[i], (*result_groupkey_4->index())[i]);
  }

  // test if the position is correct
  for (size_t i = 0; i < expect_result_3.size(); ++i) {
    EXPECT_EQ(expect_result_3[i], (*result_groupkey_4->position())[i]);
    EXPECT_EQ(expect_result_3[i], result_4->at(i));
  }
}

TEST_F(DGroupKeyTest, TestLTAndLEOperator) {
  DGroupKey<string> dgroupkey;
  dgroupkey.ConstructTreeVector(origin_data_1);
  DGroupKey<double> dgroupkey_1;
  dgroupkey_1.ConstructTreeVector(origin_data_4);

  vector<uint64_t> expect_result_1 = {4, 5, 6, 1, 3};
  vector<string> expect_dic_1 = {"apple", "charlie", "delta"};
  vector<uint64_t> expect_index_1 = {0, 1, 3, 5};

  vector<uint64_t> expect_result_2 = {4, 5, 6};
  vector<string> expect_dic_2 = {"apple", "charlie"};
  vector<uint64_t> expect_index_2 = {0, 1, 3};

  std::shared_ptr<vector<uint64_t>> result_1 =
      dgroupkey.GetLTRowIdListByValue("frank");

  shared_ptr<DGroupKey<string>> result_groupkey_1 =
      dgroupkey.GetLTGroupkeyByValue("frank");

  std::shared_ptr<vector<uint64_t>> result_2 =
      dgroupkey.GetLERowIdListByValue("delta");

  shared_ptr<DGroupKey<string>> result_groupkey_2 =
      dgroupkey.GetLEGroupkeyByValue("delta");

  std::shared_ptr<vector<uint64_t>> result_3 =
      dgroupkey.GetLTRowIdListByValue("delta");

  shared_ptr<DGroupKey<string>> result_groupkey_3 =
      dgroupkey.GetLTGroupkeyByValue("delta");

  shared_ptr<DGroupKey<double>> result_4 =
      dgroupkey_1.GetLTGroupkeyByValue(2000);

  /*for (size_t i = 0; i < result_4->dic()->Size(); ++i) {
    std::cout << "LT dic value: " << result_4->dic()->GetValueForValueId(i)
              << std::endl;
  }*/

  // test 1
  // test if the dic is correct
  for (size_t i = 0; i < expect_dic_1.size(); ++i) {
    EXPECT_EQ(expect_dic_1[i], (*result_groupkey_1->dic())[i]);
  }

  // test if the index is correct
  for (size_t i = 0; i < expect_index_1.size(); ++i) {
    EXPECT_EQ(expect_index_1[i], (*result_groupkey_1->index())[i]);
  }

  // test if the position is correct
  for (size_t i = 0; i < expect_result_1.size(); ++i) {
    EXPECT_EQ(expect_result_1[i], (*result_groupkey_1->position())[i]);
    EXPECT_EQ(expect_result_1[i], result_1->at(i));
  }

  // test2
  // test if the dic is correct
  for (size_t i = 0; i < expect_dic_1.size(); ++i) {
    EXPECT_EQ(expect_dic_1[i], (*result_groupkey_2->dic())[i]);
  }

  // test if the index is correct
  for (size_t i = 0; i < expect_index_1.size(); ++i) {
    EXPECT_EQ(expect_index_1[i], (*result_groupkey_2->index())[i]);
  }

  // test if the position is correct
  for (size_t i = 0; i < expect_result_1.size(); ++i) {
    EXPECT_EQ(expect_result_1[i], (*result_groupkey_2->position())[i]);
    EXPECT_EQ(expect_result_1[i], result_2->at(i));
  }

  // test3
  // test if the dic is correct
  for (size_t i = 0; i < expect_dic_2.size(); ++i) {
    EXPECT_EQ(expect_dic_2[i], (*result_groupkey_3->dic())[i]);
  }

  // test if the index is correct
  for (size_t i = 0; i < expect_index_2.size(); ++i) {
    EXPECT_EQ(expect_index_2[i], (*result_groupkey_3->index())[i]);
  }

  // test if the position is correct
  for (size_t i = 0; i < expect_result_2.size(); ++i) {
    EXPECT_EQ(expect_result_2[i], (*result_groupkey_3->position())[i]);
    EXPECT_EQ(expect_result_2[i], result_3->at(i));
  }
}

TEST_F(DGroupKeyTest, TestEQAndNEOperator) {
  DGroupKey<string> dgroupkey;
  dgroupkey.ConstructTreeVector(origin_data_1);

  vector<uint64_t> expect_result_1 = {5, 6};
  vector<uint64_t> expect_result_2 = {4, 1, 3, 2, 0, 7};
  vector<uint64_t> expect_index_2 = {0, 1, 3, 4, 5, 6};

  vector<uint64_t> expect_result_3 = {5, 6, 1, 3, 2, 0, 7};

  std::shared_ptr<vector<uint64_t>> result_1 =
      dgroupkey.GetEQRowIdListByValue("charlie");

  shared_ptr<DGroupKey<string>> result_groupkey_1 =
      dgroupkey.GetEQGroupkeyByValue("charlie");

  std::shared_ptr<vector<uint64_t>> result_2 =
      dgroupkey.GetNERowIdListByValue("charlie");

  shared_ptr<DGroupKey<string>> result_groupkey_2 =
      dgroupkey.GetNEGroupkeyByValue("charlie");

  std::shared_ptr<vector<uint64_t>> result_3 =
      dgroupkey.GetNERowIdListByValue("apple");

  shared_ptr<DGroupKey<string>> result_groupkey_3 =
      dgroupkey.GetNEGroupkeyByValue("apple");

  for (size_t i = 0; i < expect_result_1.size(); ++i) {
    EXPECT_EQ(expect_result_1[i], (*result_groupkey_1->position())[i]);
    EXPECT_EQ(expect_result_1[i], result_1->at(i));
  }

  for (size_t i = 0; i < expect_result_2.size(); ++i) {
    EXPECT_EQ(expect_result_2[i], (*result_groupkey_2->position())[i]);
    EXPECT_EQ(expect_result_2[i], result_2->at(i));
  }

  for (size_t i = 0; i < expect_index_2.size(); ++i) {
    EXPECT_EQ(expect_index_2[i], (*result_groupkey_2->index())[i]);
  }

  for (size_t i = 0; i < expect_result_3.size(); ++i) {
    EXPECT_EQ(expect_result_3[i], (*result_groupkey_3->position())[i]);
    EXPECT_EQ(expect_result_3[i], result_3->at(i));
  }
}

TEST_F(DGroupKeyTest, TestSerializeAndDeserialize) {
  DGroupKey<string> dgroupkey, dgroupkey_copy;
  DGroupKey<double> dgroupkey_double, dgroupkey_double_copy;

  string serialize_data;
  string serialize_data_double;

  dgroupkey.ConstructTreeVector(origin_data_1);
  dgroupkey.SerializeToString(serialize_data);

  dgroupkey_double.ConstructTreeVector(origin_data_4);
  dgroupkey_double.SerializeToString(serialize_data_double);

  //cout << "serialize size: " << serialize_data.capacity() << "bytes " << endl;
  //cout << "serialize size (double type): " << serialize_data_double.capacity()
  //     << "bytes" << endl;

  dgroupkey_copy.DeserializeFromString(serialize_data);
  dgroupkey_double_copy.DeserializeFromString(serialize_data_double);

  ASSERT_EQ(dgroupkey.dic()->Size(), dgroupkey_copy.dic()->Size());
  ASSERT_EQ(dgroupkey_double.dic()->Size(),
            dgroupkey_double_copy.dic()->Size());

  for (size_t i = 0; i < dgroupkey.dic()->Size(); ++i) {
    ASSERT_EQ(dgroupkey.dic()->GetValueForValueId(i),
              dgroupkey_copy.dic()->GetValueForValueId(i));
  }

  ASSERT_EQ(dgroupkey.index()->rows(), dgroupkey_copy.index()->rows());
  for (size_t i = 0; i < dgroupkey.index()->rows(); ++i) {
    ASSERT_EQ(dgroupkey.index()->Get(i), dgroupkey_copy.index()->Get(i));
  }

  ASSERT_EQ(dgroupkey.position()->rows(), dgroupkey_copy.position()->rows());
  for (size_t i = 0; i < dgroupkey.position()->rows(); ++i) {
    ASSERT_EQ(dgroupkey.position()->Get(i), dgroupkey_copy.position()->Get(i));
  }

  for (size_t i = 0; i < dgroupkey_double.dic()->Size(); ++i) {
    ASSERT_EQ(dgroupkey_double.dic()->GetValueForValueId(i),
              dgroupkey_double_copy.dic()->GetValueForValueId(i));
  }

  ASSERT_EQ(dgroupkey_double.index()->rows(),
            dgroupkey_double_copy.index()->rows());
  for (size_t i = 0; i < dgroupkey_double.index()->rows(); ++i) {
    ASSERT_EQ(dgroupkey_double.index()->Get(i),
              dgroupkey_double_copy.index()->Get(i));
  }

  ASSERT_EQ(dgroupkey_double.position()->rows(),
            dgroupkey_double_copy.position()->rows());
  for (size_t i = 0; i < dgroupkey_double.position()->rows(); ++i) {
    ASSERT_EQ(dgroupkey_double.position()->Get(i),
              dgroupkey_double_copy.position()->Get(i));
  }
}

TEST_F(DGroupKeyTest, TestSplit) {
  DGroupKey<string> dgroupkey;
  dgroupkey.ConstructTreeVector(origin_data_1);

  vector<uint64_t> row_id_set = {0, 3, 5};
  vector<uint64_t> dic_entry_set = {4, 2, 1};
  vector<string> expect_dic_row_id_set = {"charlie", "delta", "hotel"};
  vector<uint64_t> expect_index_set = {0, 1, 2, 3};
  vector<uint64_t> expect_position_set = {5, 3, 0};

  DGroupKey<double> dgroupkey_double;
  dgroupkey_double.ConstructTreeVector(origin_data_5);

  DGroupKey<char> dgroupkey_char;
  dgroupkey_char.ConstructTreeVector(origin_data_6);

  std::shared_ptr<DGroupKey<string>> split_groupkey = dgroupkey.Split(0, 4);

  std::shared_ptr<DGroupKey<char>> split_groupkey_char =
      dgroupkey_char.Split(0, 3);

  std::shared_ptr<DGroupKey<char>> split_groupkey_char_1 =
      dgroupkey_char.Split(4, 5);

  split_groupkey_char_1->Print();

  std::shared_ptr<DGroupKey<string>> split_groupkey_by_rowid_set =
      std::dynamic_pointer_cast<DGroupKey<string>>(
          dgroupkey.SplitByRowIdList(row_id_set, dic_entry_set));

  // split_groupkey_by_rowid_set->Print();

  for (size_t i = 0; i < expect_dic_row_id_set.size(); ++i) {
    ASSERT_EQ(expect_dic_row_id_set[i],
              split_groupkey_by_rowid_set->dic()->GetValueForValueId(
                  i + split_groupkey_by_rowid_set->GetDicBeginEntry()));
  }

  for (size_t i = 0; i < expect_index_set.size(); ++i) {
    ASSERT_EQ(expect_index_set[i],
              split_groupkey_by_rowid_set->index()->Get(i));
  }

  for (size_t i = 0; i < expect_position_set.size(); ++i) {
    ASSERT_EQ(expect_position_set[i],
              split_groupkey_by_rowid_set->position()->Get(i));
  }

  std::shared_ptr<DGroupKey<string>> split_range_groupkey =
      dgroupkey.SplitByRange("a", "d");

  std::shared_ptr<DGroupKey<string>> split_range_groupkey_2 =
      dgroupkey.SplitByRange("b", "e");

  std::shared_ptr<DGroupKey<string>> split_range_groupkey_3 =
      dgroupkey.SplitByRange("a", "inbox");

  std::shared_ptr<DGroupKey<double>> split_range_groupkey_5 =
      dgroupkey_double.SplitByRange(0, 24);

  // split_range_groupkey_5->Print();

  vector<string> expected_dic = {"charlie", "delta"};

  ASSERT_EQ('a', split_groupkey_char->dic()->GetSmallestValue());
  ASSERT_EQ('f', split_groupkey_char->dic()->GetGreatestValue());

  for (size_t i = 0; i < split_range_groupkey_3->dic()->Size(); ++i) {
    ASSERT_EQ(dgroupkey.dic()->GetValueForValueId(i),
              split_range_groupkey_3->dic()->GetValueForValueId(
                  i + split_range_groupkey_3->GetDicBeginEntry()));
  }

  for (size_t i = 0; i < split_range_groupkey_3->index()->rows(); ++i) {
    ASSERT_EQ(dgroupkey.index()->Get(i),
              split_range_groupkey_3->index()->Get(i));
  }

  for (size_t i = 0; i < split_range_groupkey_3->position()->rows(); ++i) {
    ASSERT_EQ(dgroupkey.position()->Get(i),
              split_range_groupkey_3->position()->Get(i));
  }
  for (size_t i = 0; i < split_range_groupkey_2->dic()->Size(); ++i) {
    ASSERT_EQ(expected_dic[i],
              split_range_groupkey_2->dic()->GetValueForValueId(
                  i + split_range_groupkey_2->GetDicBeginEntry()));
  }

  for (size_t i = 0; i < split_groupkey->dic()->Size(); ++i) {
    ASSERT_EQ(dgroupkey.dic()->GetValueForValueId(i),
              split_groupkey->dic()->GetValueForValueId(
                  i + split_groupkey->GetDicBeginEntry()));
  }

  for (size_t i = 0; i < split_groupkey->index()->rows(); ++i) {
    ASSERT_EQ(dgroupkey.index()->Get(i), split_groupkey->index()->Get(i));
  }

  for (size_t i = 0; i < split_groupkey->position()->rows(); ++i) {
    ASSERT_EQ(dgroupkey.position()->Get(i), split_groupkey->position()->Get(i));
  }

  for (size_t i = 0; i < split_range_groupkey->dic()->Size(); ++i) {
    ASSERT_EQ(dgroupkey.dic()->GetValueForValueId(i),
              split_range_groupkey->dic()->GetValueForValueId(
                  i + split_range_groupkey->GetDicBeginEntry()));
  }

  for (size_t i = 0; i < split_range_groupkey->index()->rows(); ++i) {
    ASSERT_EQ(dgroupkey.index()->Get(i), split_range_groupkey->index()->Get(i));
  }

  for (size_t i = 0; i < split_range_groupkey->position()->rows(); ++i) {
    ASSERT_EQ(dgroupkey.position()->Get(i),
              split_range_groupkey->position()->Get(i));
  }
}

TEST_F(DGroupKeyTest, TestSplitByDicEntry) {
  DGroupKey<string> dgroupkey;
  dgroupkey.ConstructTreeVector(origin_data_1);
  vector<uint64_t> dic_entry_list = {0, 1, 3, 5, 6, 7, 8, 9, 10};
  vector<uint64_t> dic_entry_list_2 = {1, 2, 4};

  vector<string> expect_dic = {"apple", "charlie", "frank"};
  vector<uint64_t> expect_position = {4, 5, 6, 2};

  std::shared_ptr<DGroupKey<string>> ret =
      std::dynamic_pointer_cast<DGroupKey<string>>(
          dgroupkey.SplitByDicEntryList(dic_entry_list));

  for (size_t i = 0; i < expect_dic.size(); ++i) {
    ASSERT_EQ(expect_dic[i],
              ret->dic()->GetValueForValueId(i + ret->GetDicBeginEntry()));
  }

  for (size_t i = 0; i < expect_position.size(); ++i) {
    ASSERT_EQ(expect_position[i], ret->position()->Get(i));
  }

  dgroupkey.SetDicBeginEntry(1);

  std::shared_ptr<DGroupKey<string>> ret2 =
      std::dynamic_pointer_cast<DGroupKey<string>>(
          dgroupkey.SplitByDicEntryList(dic_entry_list_2));

  for (size_t i = 0; i < expect_dic.size(); ++i) {
    ASSERT_EQ(expect_dic[i],
              ret2->dic()->GetValueForValueId(i + ret2->GetDicBeginEntry()));
  }

  for (size_t i = 0; i < expect_position.size(); ++i) {
    ASSERT_EQ(expect_position[i], ret2->position()->Get(i));
  }
}

TEST_F(DGroupKeyTest, TestAppendGroupKey) {
  vector<string> append_data_1 = {"a", "a", "b", "c", "d"};
  vector<string> append_data_2 = {"e", "e", "f", "f", "g"};

  vector<string> expect_dic = {"a", "b", "c", "d", "e", "f", "g"};
  vector<uint64_t> expect_index = {0, 2, 3, 4, 5, 7, 9, 10};
  vector<uint64_t> expect_position = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  std::shared_ptr<DGroupKey<string>> dgroupkey_1 =
      std::make_shared<DGroupKey<string>>();
  std::shared_ptr<DGroupKey<string>> dgroupkey_2 =
      std::make_shared<DGroupKey<string>>();

  dgroupkey_1->ConstructTreeVector(append_data_1);
  dgroupkey_2->ConstructTreeVector(append_data_2, append_data_1.size());

  // dgroupkey_1->Print();
  // dgroupkey_2->Print();

  dgroupkey_1->AppendGroupKey(dgroupkey_2);

  for (size_t i = 0; i < expect_dic.size(); ++i) {
    ASSERT_EQ(expect_dic[i], dgroupkey_1->dic()->GetValueForValueId(i));
  }

  for (size_t i = 0; i < expect_index.size(); ++i) {
    ASSERT_EQ(expect_index[i], dgroupkey_1->index()->Get(i));
  }

  for (size_t i = 0; i < expect_position.size(); ++i) {
    ASSERT_EQ(expect_position[i], dgroupkey_1->position()->Get(i));
  }
}
