#include "BitCompressedVector.h"
#include <gtest/gtest.h>
#include <boost/timer.hpp>
#include <vector>

#include <cmath>
#include <iostream>
#include <memory>

using namespace std;

class BitCompressedVectorTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    origin_data.resize(2001);

    for (auto i = 0; i < 2000; ++i) {
      origin_data[i] = i;
    }

    int bits = (int)log2(origin_data.size()) + 1;

    // compressed_vector origin data
    compresed_vector =
        std::make_shared<BitCompressedVector<int> >(origin_data.size(), bits);

    for (size_t i = 0; i < origin_data.size(); ++i) {
      compresed_vector->Set(i, origin_data[i]);
    }
  }

  virtual void TearDown() {}

 protected:
  vector<int> origin_data;
  shared_ptr<BitCompressedVector<int> > compresed_vector;
};

template <typename T>
bool CompareVector(shared_ptr<BitCompressedVector<T> > &compress,
                   vector<int> &data) {
  boost::timer t;
  for (size_t i = 0; i < data.size(); ++i) {
    auto value = compress->Get(i);

    if (value != data[i]) {
      cout << value << " " << data[i] << endl;
      return false;
    }
  }
  //cout << "now time elapse " << t.elapsed() << "s" << endl;

  return true;
}

template <typename T>
bool CompareCompressedVector(shared_ptr<BitCompressedVector<T> > &first,
                             shared_ptr<BitCompressedVector<T> > &second) {
  if (first->rows() != second->rows()) return false;

  int rows = first->rows();

  for (auto i = 0; i < rows; ++i) {
    if (first->Get(i) != second->Get(i)) return false;
  }

  return true;
}

TEST_F(BitCompressedVectorTest, TestSetAndGet) {
  EXPECT_TRUE(CompareVector<int>(compresed_vector, origin_data));
}

TEST_F(BitCompressedVectorTest, TestCopy) {
  // the smart point cast
  shared_ptr<BitCompressedVector<int> > copy_vector =
      std::dynamic_pointer_cast<BitCompressedVector<int> >(
          compresed_vector->Copy());

  EXPECT_TRUE(CompareCompressedVector(copy_vector, compresed_vector));
  EXPECT_TRUE(copy_vector.get() != compresed_vector.get());
}

TEST_F(BitCompressedVectorTest, TestResizeExpandBitWidth) {
  uint64_t bits = compresed_vector->bits();
  compresed_vector->Resize(4000, int(log2(4000)) + 1);

  for (size_t i = 0; i < 2000; ++i) {
    EXPECT_EQ(i, compresed_vector->Get(i));
  }

  EXPECT_NE(bits, compresed_vector->bits());
  EXPECT_EQ(12, compresed_vector->bits());
  EXPECT_EQ(4000, compresed_vector->rows());
  EXPECT_EQ(sizeof(uint64_t) * ((4000 * 12 / (sizeof(uint64_t) * 8)) + 1),
            compresed_vector->size());

  int value = 2000;

  for (size_t i = 2000; i < 4000; ++i) {
    compresed_vector->Set(i, value);
    value++;
  }

  for (size_t i = 2000; i < 4000; ++i) {
    EXPECT_EQ(i, compresed_vector->Get(i));
  }
}

TEST_F(BitCompressedVectorTest, TestResizeNoExpanedBitWidth) {
  uint64_t bits = compresed_vector->bits();
  uint64_t *data_ptr = compresed_vector->data();

  compresed_vector->Resize(1004);

  // no bits expand so it needed expanded the width
  EXPECT_EQ(bits, compresed_vector->bits());
  EXPECT_EQ(data_ptr, compresed_vector->data());
}

TEST_F(BitCompressedVectorTest, TestSerializeAndDeserialize) {
  std::string data;
  compresed_vector->SerializeToString(data);

  BitCompressedVector<int> result_vector;
  result_vector.DeserializeFromString(data);

  //cout << data.capacity() << endl;
  //cout << compresed_vector->size() << endl;

  ASSERT_EQ(compresed_vector->size(), result_vector.size());
  ASSERT_EQ(compresed_vector->bits(), result_vector.bits());
  ASSERT_EQ(compresed_vector->rows(), result_vector.rows());

  for (size_t i = 0; i < compresed_vector->rows(); ++i) {
    ASSERT_EQ(compresed_vector->Get(i), result_vector.Get(i));
  }
}

TEST_F(BitCompressedVectorTest, TestSize) {
  int bits50000 = (int)(log2(50000)) + 1;

  //cout << "bits width:" << bits50000 << endl;
  BitCompressedVector<uint64_t> result_vector(50000, bits50000);

  for (size_t i = 0; i < 50000; ++i) {
    result_vector.Set(i, i);
  }

  //cout << "blocks number " << result_vector.blocks() << endl;
  //cout << "vector size: " << result_vector.size() << endl;
}
