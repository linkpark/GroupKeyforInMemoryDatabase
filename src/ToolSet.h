/*
 * =================================================================
 *
 *            Filename:    ToolSet.h
 *
 *         Description:    the tool function set
 *
 *             Version:    v1.0
 *             Created:    2015-11-11 10:36
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */

#ifndef TOOLSET_H_
#define TOOLSET_H_

#include <string.h>
#include <sys/types.h>

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "DGroupKey.h"

class ToolSet {
 public:
   typedef std::unordered_map< uint64_t, uint64_t > FourthVectorType;
  template <class T>
  static void VectorSerializeToString(std::vector<T> &origin_data,
                                      std::string &result) {
    size_t length = origin_data.size();
    result.append((char *)(&length), sizeof(size_t));

    for (size_t i = 0; i < origin_data.size(); ++i) {
      result.append((char *)(&origin_data[i]), sizeof(T));
    }
  }

  template <class T>
  static void VectorDeserializeFromString(
      std::shared_ptr<std::vector<T> > &result, std::string &data) {
    if (result == NULL) {
      result = std::make_shared<std::vector<T> >();
    }

    const char *cursor = data.data();
    size_t length(0);
    ::memcpy(&length, cursor, sizeof(size_t));
    cursor += sizeof(size_t);

    T tmp_value;
    for (size_t i = 0; i < length; ++i) {
      ::memcpy(&tmp_value, cursor, sizeof(T));
      cursor += sizeof(T);
      result->push_back(tmp_value);
    }
  }

  template< class T >
  static std::shared_ptr< FourthVectorType > GenerateForthVector(std::shared_ptr<DGroupKey<T> > groupkey_slice) {
    // Generate Fourth Vector
    std::shared_ptr<BitCompressedVector<uint64_t> > slice_index =
        groupkey_slice->index();
    std::shared_ptr<BitCompressedVector<uint64_t> > slice_position =
        groupkey_slice->position();
    std::shared_ptr<std::unordered_map<uint64_t, uint64_t> > fourth_vector_ptr(
        new std::unordered_map<uint64_t, uint64_t>(slice_position->rows()));
    uint64_t index = 0;
    uint64_t position_index = 0;
    uint64_t dic_index = 0;
    // Scan Index Vector by group, generate Fourth Vector
    // The entry in Fourth Vector begins from 0
    for (; index < slice_index->rows() - 1; ++index) {
      uint64_t begin = slice_index->Get(index);
      uint64_t end = slice_index->Get(index + 1);

      for (position_index = begin; position_index < end; ++position_index) {
        fourth_vector_ptr->insert(
            {slice_position->Get(position_index), dic_index});
      }
      ++dic_index;
    }

    return fourth_vector_ptr;
  }
};

#endif
