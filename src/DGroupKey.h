/*
 * =================================================================
 *
 *            Filename:    DGroupKey.h
 *
 *         Description:    the groupkey structure, which is the core
 *                      store unit of column store
 *             Version:    v1.0
 *             Created:    2015-09-19 15:46
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */
#ifndef DGROUPKEY_H_
#define DGROUPKEY_H_
#include "AbstractIndex.h"
#include "BitCompressedVector.h"
#include "OrderedDictionary.h"

#include <algorithm>
#include <bitset>
#include <boost/timer.hpp>
#include <map>
#include <typeinfo>
#include <utility>
#include <vector>
#include <unordered_set>

template <class T>
class DGroupKey : public AbstractIndex {
 private:
  typedef std::pair<T, uint64_t> dic_pair;

 public:
  DGroupKey() { dic_ = std::make_shared<OrderedDictionary<T>>(); }

  // construct groupkey by three vector which has been already build
  DGroupKey(std::shared_ptr<OrderedDictionary<T>> dic_ptr,
            std::shared_ptr<BitCompressedVector<uint64_t>> index_offset,
            std::shared_ptr<BitCompressedVector<uint64_t>> position)
      : position_(position), index_(index_offset), dic_(dic_ptr) {}

  virtual ~DGroupKey() {}

  void AppendGroupKey(std::shared_ptr<DGroupKey<T>> &append_groupkey) {
    int bits = std::max(position_->bits(), append_groupkey->position()->bits());

    std::shared_ptr<BitCompressedVector<uint64_t>> new_index_ptr =
        std::make_shared<BitCompressedVector<uint64_t>>(
            index_->rows() + append_groupkey->index()->rows() - 1, bits);

    std::shared_ptr<BitCompressedVector<uint64_t>> new_position_ptr =
        std::make_shared<BitCompressedVector<uint64_t>>(
            position_->rows() + append_groupkey->position()->rows(), bits);

    // set dictionary
    dic_->AppendDic(*(append_groupkey->dic()));

    // set index vector
    for (size_t i = 0; i < (index_->rows() - 1); ++i) {
      uint64_t tmp = index_->Get(i);
      new_index_ptr->Set(i, tmp);
    }

    uint64_t delta_index_value = index_->Get((index_->rows() - 1));

    for (size_t i = 0; i < append_groupkey->index()->rows(); ++i) {
      uint64_t tmp = append_groupkey->index()->Get(i) + delta_index_value;
      new_index_ptr->Set((i + index_->rows() - 1), tmp);
    }

    // set position vector
    for (size_t i = 0; i < position_->rows(); ++i) {
      uint64_t tmp = position_->Get(i);
      new_position_ptr->Set(i, tmp);
    }

    for (size_t i = 0; i < append_groupkey->position()->rows(); ++i) {
      uint64_t tmp = append_groupkey->position()->Get(i);
      new_position_ptr->Set((i + position_->rows()), tmp);
    }

    index_ = new_index_ptr;
    position_ = new_position_ptr;

    return;
  }

  std::shared_ptr<AbstractIndex> SplitByDicEntryList(
      std::vector<uint64_t> &dict_entry_list) {
    if (dict_entry_list.size() == 0) {
      return NULL;
    }

    size_t begin_entry = GetDicBeginEntry();

    // std::sort( dict_entry_list.begin(), dict_entry_list.end() );
    // sort by bitset
    uint64_t dict_size = dic_->Size();
    BitCompressedVector<uint64_t> sort_bit_vector(dict_size, 1);

    for (size_t i = 0; i < dict_entry_list.size(); ++i) {
      if (dict_entry_list[i] >= GetDicBeginEntry()) {
        size_t dict_position = dict_entry_list[i] - begin_entry;

        if (dict_position >= dic_->Size()) continue;

        if (sort_bit_vector[dict_position] != 1) {
          uint64_t result = 1;
          sort_bit_vector.Set(dict_position, result);
        }
      }
    }

    std::vector<uint64_t> cut_dict_entry_list;
    for (size_t i = 0; i < sort_bit_vector.rows(); ++i) {
      if (sort_bit_vector[i] == 1) {
        cut_dict_entry_list.push_back((i + begin_entry));
      }
    }

    if (cut_dict_entry_list.size() == 0) return NULL;

    std::shared_ptr<OrderedDictionary<T>> new_dic_ptr =
        std::make_shared<OrderedDictionary<T>>();

    size_t index_rows = cut_dict_entry_list.size() + 1;
    std::shared_ptr<BitCompressedVector<uint64_t>> new_index_ptr =
        std::make_shared<BitCompressedVector<uint64_t>>(index_rows,
                                                        index_->bits());

    std::vector<uint64_t> tmp_position;
    tmp_position.reserve(position_->rows());

    size_t m(0), n(0);
    size_t origin_dic_size = dic_->Size();
    size_t new_dic_size = cut_dict_entry_list.size();
    size_t index_offset_position(0);

    while (m < origin_dic_size && n < new_dic_size) {
      if (m == (cut_dict_entry_list[n] - begin_entry)) {
        new_dic_ptr->AddValue(GetDicValueById((m + begin_entry)));

        // add index offset vector
        new_index_ptr->Set(n, index_offset_position);

        size_t interval_numbers = index_->Get(m + 1) - index_->Get(m);
        index_offset_position += interval_numbers;

        // add posting vector
        for (size_t i = index_->Get(m); i < index_->Get(m + 1); ++i) {
          tmp_position.push_back(position_->Get(i));
        }

        ++n;
      }

      ++m;
    }

    // add the last element of the new_index
    new_index_ptr->Set(n, index_offset_position);

    std::shared_ptr<BitCompressedVector<uint64_t>> new_posting_ptr =
        std::make_shared<BitCompressedVector<uint64_t>>(tmp_position.size(),
                                                        position_->bits());

    for (size_t i = 0; i < tmp_position.size(); ++i) {
      new_posting_ptr->Set(i, tmp_position[i]);
    }

    std::shared_ptr<DGroupKey<T>> group_key_ptr =
        std::make_shared<DGroupKey<T>>(new_dic_ptr, new_index_ptr,
                                       new_posting_ptr);

    std::shared_ptr<AbstractIndex> ret =
        std::dynamic_pointer_cast<AbstractIndex>(group_key_ptr);

    return ret;
  }

  /* *
   * @describe split the groupkey structure by lower bound and upper bound
   * @param[in] begin, the lower bound
   * @param[in] end, the upper bound
   *
   * @return the shared_ptr< DGroupKey<T> > about the split result
   * */
  std::shared_ptr<DGroupKey<T>> Split(uint64_t begin, uint64_t end) {
    // the input is illegal
    if (end <= begin) return NULL;

    std::shared_ptr<OrderedDictionary<T>> dic = dic_->Split(begin, end);

    size_t index_offset_begin = begin - dic_->entry_begin();
    size_t index_offset_end = end - dic_->entry_begin();
    // attention that index vector should split from (begin to end+1 )
    std::shared_ptr<BitCompressedVector<uint64_t>> index =
        index_->Split(index_offset_begin, index_offset_end + 1);

    uint64_t begin_value = (*index)[0];
    for (size_t i = 0; i < index->rows(); ++i) {
      uint64_t tmp_value = (*index)[i] - begin_value;
      index->Set(i, tmp_value);
    }

    std::shared_ptr<BitCompressedVector<uint64_t>> position = position_->Split(
        (*index_)[index_offset_begin], (*index_)[index_offset_end]);

    std::shared_ptr<DGroupKey<T>> ret =
        std::make_shared<DGroupKey<T>>(dic, index, position);

    return ret;
  }

  std::shared_ptr<AbstractIndex> SplitByRowIdList( std::unordered_set<uint64_t> &row_id_list ) {
    std::shared_ptr< DGroupKey<T> > ret = std::make_shared< DGroupKey<T> >();
    std::vector< std::pair<T, uint64_t> > tmp_data;

    for( size_t i = 0; i < dic_->Size(); ++i ) {
      for( size_t j = (*index_)[i]; j < (*index_)[i+1]; ++j ) { 
        if( row_id_list.find( position_->Get(j)) != row_id_list.end() )
            tmp_data.push_back( {(*dic_)[i], position_->Get(j)} );
      }
    }

    ret->ConstructThreeVector( tmp_data );
    return ret;
  }

  std::shared_ptr<AbstractIndex> SplitByRowIdList(
      std::vector<uint64_t> &row_id_list, std::vector<uint64_t> &dict_id_list) {
    if (row_id_list.size() == 0 || dict_id_list.size() == 0) {
      LOG(INFO) << "SplitByRowIdList: the row id list is empty!";
      return NULL;
    }

    std::multimap<T, uint64_t> ordered_dic;

    for (size_t i = 0; i < row_id_list.size(); ++i) {
      T dic_value = dic_->GetValueForValueId((dict_id_list[i]));
      ordered_dic.insert(std::make_pair(dic_value, row_id_list[i]));
    }

    uint64_t max_row_id =
        *std::max_element(row_id_list.begin(), row_id_list.end());
    size_t position_bits = (size_t)(log2(max_row_id)) + 1;
    size_t index_bits = (size_t)(log2(row_id_list.size())) + 1;

    std::vector<uint64_t> tmp_index;

    std::shared_ptr<OrderedDictionary<T>> new_dict =
        std::make_shared<OrderedDictionary<T>>();

    std::shared_ptr<BitCompressedVector<uint64_t>> new_position =
        std::make_shared<BitCompressedVector<uint64_t>>(row_id_list.size(),
                                                        position_bits);

    T tmp;
    size_t position = 0;

    for (auto it = ordered_dic.begin(); it != ordered_dic.end(); ++it) {
      if (it->first != tmp) {
        tmp = it->first;
        new_dict->AddValue(tmp);
        tmp_index.push_back(position);
      }

      new_position->Set(position, it->second);
      position++;
    }

    tmp_index.push_back(row_id_list.size());

    std::shared_ptr<BitCompressedVector<uint64_t>> new_index =
        std::make_shared<BitCompressedVector<uint64_t>>(tmp_index.size(),
                                                        index_bits);

    for (size_t i = 0; i < tmp_index.size(); ++i) {
      new_index->Set(i, tmp_index[i]);
    }

    std::shared_ptr<DGroupKey<T>> ret =
        std::make_shared<DGroupKey<T>>(new_dict, new_index, new_position);

    return ret;
  }

  std::shared_ptr<DGroupKey<T>> SplitByRange(T lower_bound, T upper_bound) {
    uint64_t lower_bound_position, upper_bound_position;
    if (lower_bound <= dic_->GetSmallestValue()) {
      lower_bound_position = dic_->entry_begin();
    } else if (lower_bound > dic_->GetGreatestValue()) {
      return NULL;
    } else {
      // O ( log2(n) )
      lower_bound_position = dic_->FindLowerBoundForValue(lower_bound);
    }

    if (upper_bound >= dic_->GetGreatestValue()) {
      upper_bound_position = dic_->GetEndEntry() + 1;
    } else if (upper_bound < dic_->GetSmallestValue()) {
      return NULL;
    } else {
      // O ( log2(n) )
      upper_bound_position = dic_->FindUpperBoundForValue(upper_bound);
    }

    // O(n)
    std::shared_ptr<DGroupKey<T>> ret;
    try {
      ret = Split(lower_bound_position, upper_bound_position);
      LOG(INFO) << " upper bound: " << upper_bound_position;
      LOG(INFO) << " lower bound: " << lower_bound_position;
      LOG(INFO) << " dict begin entry " << dic_->entry_begin();
      LOG(INFO) << " dict end entry " << dic_->GetEndEntry();
    } catch (...) {
      LOG(ERROR) << " upper bound: " << upper_bound_position;
      LOG(ERROR) << " lower bound: " << lower_bound_position;
      LOG(ERROR) << " dict begin entry " << dic_->entry_begin();
      LOG(ERROR) << " dict end entry " << dic_->GetEndEntry();
      throw std::runtime_error("SplitByRange runtime error! beyond bound!");
    }

    return ret;
  }

  void ConstructThreeVector(OrderedDictionary<std::string> &origin_data,
                           uint64_t begin_row_id = 0) {
    std::vector<dic_pair> ordered_dic;
    uint64_t row_id;

    for (uint64_t i = 0; i < origin_data.Size(); ++i) {
      row_id = i + begin_row_id;
      ordered_dic.push_back(std::make_pair(origin_data[i], row_id));
      // ordered_dic.insert(std::make_pair(origin_data[i], i));
    }

    std::sort(ordered_dic.begin(), ordered_dic.end());

    // std::cout << "sort timer " << t.elapsed() << "s" << std::endl;

    int bits = (int)(log2(origin_data.Size() + begin_row_id)) + 1;
    position_ = std::make_shared<BitCompressedVector<uint64_t>>(
        origin_data.Size(), bits);

    size_t i = 0;
    size_t index_offset = 0;
    std::vector<size_t> index_vector;
    T before;

    for (auto ix = ordered_dic.begin(); ix != ordered_dic.end(); ++ix) {
      if (ix == ordered_dic.begin()) {
        dic_->AddValue(ix->first);
        before = ix->first;
        index_vector.push_back(index_offset);
      }

      if (ix->first != before) {
        dic_->AddValue(ix->first);
        before = ix->first;
        index_vector.push_back(index_offset);
      }

      index_offset++;
      position_->Set(i, ix->second);
      i++;
    }

    // to marked the endline
    index_vector.push_back(index_offset);

#ifdef DBUGGROUPKEY
// std::cout << "build time " << t.elapsed() << "s" << std::endl;
#endif
    // build index offset
    index_ = std::make_shared<BitCompressedVector<uint64_t>>(
        index_vector.size(), bits);

    for (size_t i = 0; i < index_vector.size(); ++i) {
      index_->Set(i, index_vector[i]);
    }

#ifdef DBUGGROUPKEY
// std::cout << "finished time " << t.elapsed() << "s" << std::endl;
#endif
  }

  /* *
   * @describe Construct Three Vector of DGroupKey
   *
   * @param[in] origin_data, the origin data list;
   * @param[in] begin_row_id, the begin row id's value
   * */
  void ConstructThreeVector(std::vector<T> &origin_data,
                           uint64_t begin_row_id = 0) {
    std::vector<dic_pair> ordered_dic;
    ordered_dic.reserve((origin_data.size() + 1) * sizeof(T));
    uint64_t row_id;
    // std::multimap< T, uint64_t > ordered_dic;
    // boost::timer t;

    // O(NlogN) time complexity
    for (uint64_t i = 0; i < origin_data.size(); ++i) {
      row_id = i + begin_row_id;
      ordered_dic.push_back(std::make_pair(origin_data[i], row_id));
      // ordered_dic.insert(std::make_pair(origin_data[i], i));
    }

    // std::cout << "import timer " << t.elapsed() << "s" << std::endl;
    // O(nlogn);
    std::sort(ordered_dic.begin(), ordered_dic.end());

    // std::cout << "sort timer " << t.elapsed() << "s" << std::endl;

    int bits = (int)(log2(origin_data.size() + begin_row_id)) + 1;
    position_ = std::make_shared<BitCompressedVector<uint64_t>>(
        origin_data.size(), bits);

    size_t i = 0;
    size_t index_offset = 0;
    std::vector<size_t> index_vector;
    T before;

    for (auto ix = ordered_dic.begin(); ix != ordered_dic.end(); ++ix) {
      if (ix == ordered_dic.begin()) {
        dic_->AddValue(ix->first);
        before = ix->first;
        index_vector.push_back(index_offset);
      }

      if (ix->first != before) {
        dic_->AddValue(ix->first);
        before = ix->first;
        index_vector.push_back(index_offset);
      }

      index_offset++;
      position_->Set(i, ix->second);
      i++;
    }

    // to marked the endline
    index_vector.push_back(index_offset);

#ifdef DBUGGROUPKEY
// std::cout << "build time " << t.elapsed() << "s" << std::endl;
#endif
    // build index offset
    index_ = std::make_shared<BitCompressedVector<uint64_t>>(
        index_vector.size(), bits);

    for (size_t i = 0; i < index_vector.size(); ++i) {
      index_->Set(i, index_vector[i]);
    }

#ifdef DBUGGROUPKEY
// std::cout << "finished time " << t.elapsed() << "s" << std::endl;
#endif
  }

  /* *
   * @describe Construct Three Vector of DGroupKey
   *
   * @param[in] origin_data, the origin data list;
   * @param[in] begin_row_id, the begin row id's value
   * */
  void ConstructThreeVector(std::vector<std::pair<T, uint64_t>> &ordered_dic,
                           uint64_t begin_row_id = 0) {
    std::sort(ordered_dic.begin(), ordered_dic.end());

    int bits = (int)(log2(ordered_dic.size() + begin_row_id)) + 1;
    position_ = std::make_shared<BitCompressedVector<uint64_t>>(
        ordered_dic.size(), bits);

    size_t i = 0;
    size_t index_offset = 0;
    std::vector<size_t> index_vector;
    T before;

    for (auto ix = ordered_dic.begin(); ix != ordered_dic.end(); ++ix) {
      if (ix == ordered_dic.begin()) {
        dic_->AddValue(ix->first);
        before = ix->first;
        index_vector.push_back(index_offset);
      }

      if (ix->first != before) {
        dic_->AddValue(ix->first);
        before = ix->first;
        index_vector.push_back(index_offset);
      }

      index_offset++;
      position_->Set(i, ix->second);
      i++;
    }

    // to marked the endline
    index_vector.push_back(index_offset);

#ifdef DBUGGROUPKEY
// std::cout << "build time " << t.elapsed() << "s" << std::endl;
#endif
    // build index offset
    index_ = std::make_shared<BitCompressedVector<uint64_t>>(
        index_vector.size(), bits);

    for (size_t i = 0; i < index_vector.size(); ++i) {
      index_->Set(i, index_vector[i]);
    }

#ifdef DBUGGROUPKEY
// std::cout << "finished time " << t.elapsed() << "s" << std::endl;
#endif
  }

  std::shared_ptr<std::vector<T>> GetDicValueByIdList(
      std::vector<uint64_t> &dic_id_list) {
    std::shared_ptr<std::vector<T>> result = std::make_shared<std::vector<T>>();
    for (auto ix : dic_id_list) {
      result->push_back(dic_->GetValueForValueId(ix));
    }

    return result;
  }

  T GetDicValueById(uint64_t id) { return dic_->GetValueForValueId(id); }

  // get rowkey list by many value_list
  std::shared_ptr<std::vector<uint64_t>> GetRowIdListByValueList(
      std::vector<T> &value_list) {
    std::shared_ptr<std::vector<uint64_t>> result =
        std::make_shared<std::vector<uint64_t>>();
    std::vector<uint64_t> index_offset;
    uint64_t tmp_index_entry(0);

    for (auto ix : value_list) {
      tmp_index_entry = dic_->GetValueIdForValue(ix) - dic_->entry_begin();
      index_offset.push_back(tmp_index_entry);
    }

    // looking rowid by index offset vector and position vector;
    for (auto ix : index_offset) {
      for (int i = (*index_)[ix]; i < (*index_)[ix + 1]; ++i) {
        result->push_back((*position_)[i]);
      }
    }

    return result;
  }

  // get rowkey list by many value_list
  std::shared_ptr<std::vector<uint64_t>> GetRowIdListByDicEntryList(
      std::vector<uint64_t> &entry_list) {
    std::shared_ptr<std::vector<uint64_t>> result =
        std::make_shared<std::vector<uint64_t>>();
    std::vector<uint64_t> index_offset;
    uint64_t tmp_index_entry(0);

    for (auto ix : entry_list) {
      tmp_index_entry = ix - dic_->entry_begin();
      index_offset.push_back(tmp_index_entry);
    }

    // looking rowid by index offset vector and position vector;
    for (auto ix : index_offset) {
      for (int i = (*index_)[ix]; i < (*index_)[ix + 1]; ++i) {
        result->push_back((*position_)[i]);
      }
    }

    return result;
  }

  /* *
   * @describe get eq row id list by value
   * @param[in] base value t( ==t )
   *
   * @return result id list , if no value return NULL
   * */
  std::shared_ptr<std::vector<uint64_t>> GetEQRowIdListByValue(T value) {
    std::shared_ptr<std::vector<uint64_t>> result = NULL;
    uint64_t index_offset;

    try {
      // O(logN) binary search
      index_offset = dic_->GetValueIdForValue(value);
    } catch (...) {
      // if not found the value return null
      return NULL;
    }

    result = std::make_shared<std::vector<uint64_t>>();

    // looking rowid by index offset vector and position vector;
    for (size_t i = (*index_)[index_offset]; i < (*index_)[index_offset + 1];
         ++i) {
      result->push_back((*position_)[i]);
    }

    return result;
  }

  /* *
   * @describe get eq groupkey part by value
   * @param[in] base value t( ==t )
   *
   * @return result groupkey part, if no value return NULL
   * */
  std::shared_ptr<DGroupKey<T>> GetEQGroupkeyByValue(T value) {
    uint64_t index_offset;
    try {
      index_offset = dic_->GetValueIdForValue(value);
    } catch (...) {
      return NULL;
    }

    // split the eq part
    std::shared_ptr<DGroupKey<T>> ret = Split(index_offset, index_offset + 1);

    return ret;
  }

  /* *
   * @describe get ne row id list by value
   * @param[in] base value t( !=t )
   *
   * @return result id list , if no value return NULL
   * */
  std::shared_ptr<std::vector<uint64_t>> GetNERowIdListByValue(T value) {
    if (dic_->Size() == 1 && dic_->GetSmallestValue() == value) {
      return NULL;
    }

    std::shared_ptr<std::vector<uint64_t>> result =
        std::make_shared<std::vector<uint64_t>>();
    uint64_t index_offset;

    try {
      index_offset = dic_->GetValueIdForValue(value);
    } catch (...) {
      // if not found the value return all
      for (size_t i = 0; i < position_->rows(); ++i) {
        result->push_back(position_->Get(i));
      }

      return result;
    }

    // looking rowid by index offset vector and position vector;
    for (size_t i = 0; i < (*index_)[index_offset]; ++i) {
      result->push_back((*position_)[i]);
    }

    for (size_t i = (*index_)[index_offset + 1]; i < position_->rows(); ++i) {
      result->push_back((*position_)[i]);
    }

    return result;
  }

  /* *
   * @describe get ne groupkey part by value
   * @param[in] base value t( !=t )
   *
   * @return result groupkey part, if no value return NULL
   * */
  std::shared_ptr<DGroupKey<T>> GetNEGroupkeyByValue(T value) {
    if (dic_->Size() == 1 && dic_->GetSmallestValue() == value) {
      return NULL;
    }

    uint64_t index_offset;
    try {
      index_offset = dic_->GetValueIdForValue(value);
    } catch (...) {
      std::shared_ptr<DGroupKey<T>> ret = Copy();
      return ret;
    }

    uint64_t position_width =
        (*index_)[index_offset + 1] - (*index_)[index_offset];

    std::shared_ptr<OrderedDictionary<T>> ret_dic =
        std::make_shared<OrderedDictionary<T>>();

    std::shared_ptr<BitCompressedVector<uint64_t>> ret_index =
        std::make_shared<BitCompressedVector<uint64_t>>(index_->rows() - 1,
                                                        index_->bits());

    std::shared_ptr<BitCompressedVector<uint64_t>> ret_position =
        std::make_shared<BitCompressedVector<uint64_t>>(
            (position_->rows() - position_width), position_->bits());

    // copy the dic;
    for (size_t i = 0; i < index_offset; ++i) {
      ret_dic->AddValue((*dic_)[i]);
    }

    for (size_t i = (index_offset + 1); i < dic_->Size(); ++i) {
      ret_dic->AddValue((*dic_)[i]);
    }

    // copy the index
    for (size_t i = 0; i < index_offset; ++i) {
      uint64_t tmp_value = (*index_)[i];
      ret_index->Set(i, tmp_value);
    }

    // be careful for the bottom half of the index vector
    for (size_t i = (index_offset + 1); i < index_->rows(); ++i) {
      uint64_t tmp_value = (*index_)[i] - position_width;
      ret_index->Set(i - 1, tmp_value);
    }

    // copy the position
    for (size_t i = 0; i < (*index_)[index_offset]; ++i) {
      uint64_t tmp_value = (*position_)[i];
      ret_position->Set(i, tmp_value);
    }

    for (size_t i = (*index_)[index_offset + 1]; i < position_->rows(); ++i) {
      uint64_t tmp_value = (*position_)[i];
      ret_position->Set((i - position_width), tmp_value);
    }

    // construct the result
    std::shared_ptr<DGroupKey<T>> ret =
        std::make_shared<DGroupKey<T>>(ret_dic, ret_index, ret_position);

    return ret;
  }

  /* *
   * @describe get greater row id list by value
   * @param[in] base value t( >t )
   *
   * @return result id list , if no value return NULL
   * */
  std::shared_ptr<std::vector<uint64_t>> GetGTRowIdListByValue(T value) {
    std::shared_ptr<std::vector<uint64_t>> result = NULL;

    if (value < dic_->GetSmallestValue()) {
      result = std::make_shared<std::vector<uint64_t>>();
      for (size_t i = 0; i < position_->rows(); ++i) {
        result->push_back(position_->Get(i));
      }
    }

    if (value >= dic_->GetGreatestValue()) {
      return result;
    }

    // find the upper bound for value
    size_t value_id = dic_->FindUpperBoundForValue(value);

    std::vector<T> value_list;
    for (size_t i = value_id; i < dic_->Size(); ++i) {
      value_list.push_back(dic_->GetValueForValueId(i));
    }

    result = GetRowIdListByValueList(value_list);

    return result;
  }

  /* *
   * @describe get gt groupkey part by value
   * @param[in] base value t( >t )
   *
   * @return result groupkey part, if no value return NULL
   * */
  std::shared_ptr<DGroupKey<T>> GetGTGroupkeyByValue(T value) {
    // if the value is less than the smallest value of the dic return the copy
    if (value < dic_->GetSmallestValue()) {
      return Copy();
    }

    if (value >= dic_->GetGreatestValue()) {
      return NULL;
    }

    // find the upper bound for value
    size_t value_id = dic_->FindUpperBoundForValue(value);

    return Split(value_id, dic_->Size());
  }

  /* *
   * @describe get ge row id list by value
   * @param[in] base value t( >=t )
   *
   * @return result id list , if no value return NULL
   * */
  std::shared_ptr<std::vector<uint64_t>> GetGERowIdListByValue(T value) {
    std::shared_ptr<std::vector<uint64_t>> result = NULL;

    if (value <= dic_->GetSmallestValue()) {
      result = std::make_shared<std::vector<uint64_t>>();
      for (size_t i = 0; i < position_->rows(); ++i) {
        result->push_back(position_->Get(i));
      }
    }

    if (value > dic_->GetGreatestValue()) {
      return result;
    }

    // find the lower bound for value
    size_t value_id = dic_->FindLowerBoundForValue(value);

    std::vector<T> value_list;
    for (size_t i = value_id; i < dic_->Size(); ++i) {
      value_list.push_back(dic_->GetValueForValueId(i));
    }

    result = GetRowIdListByValueList(value_list);

    return result;
  }

  /* *
   * @describe get ge groupkey part by value
   * @param[in] base value t( >=t )
   *
   * @return result groupkey part, if no value return NULL
   * */
  std::shared_ptr<DGroupKey<T>> GetGEGroupkeyByValue(T value) {
    // if the value is less than the smallest value of the dic return the copy
    if (value <= dic_->GetSmallestValue()) {
      return Copy();
    }

    if (value > dic_->GetGreatestValue()) {
      return NULL;
    }

    // find the lower bound for value
    size_t value_id = dic_->FindLowerBoundForValue(value);

    return Split(value_id, dic_->Size());
  }

  /* *
   * @describe get row id list, which value is less than target value
   * @param[in] base value t( <t )
   *
   * @return result id list, if no value, return NULL
   * */
  std::shared_ptr<std::vector<uint64_t>> GetLTRowIdListByValue(T value) {
    std::shared_ptr<std::vector<uint64_t>> result = NULL;

    if (value > dic_->GetGreatestValue()) {
      result = std::make_shared<std::vector<uint64_t>>();
      for (size_t i = 0; i < position_->rows(); ++i) {
        result->push_back(position_->Get(i));
      }
    }

    if (value <= dic_->GetSmallestValue()) {
      return result;
    }

    // find the lower bound for value
    size_t value_id = dic_->FindLowerBoundForValue(value);

    std::vector<T> value_list;
    for (size_t i = 0; i < value_id; ++i) {
      value_list.push_back(dic_->GetValueForValueId(i));
    }

    result = GetRowIdListByValueList(value_list);

    return result;
  }

  std::shared_ptr<std::vector<uint64_t>> GetRowIdListByDicId(uint64_t dic_id) {
    uint64_t dic_entry = dic_id - dic_->entry_begin();
    std::shared_ptr<std::vector<uint64_t>> ret =
        std::make_shared<std::vector<uint64_t>>();

    for (uint64_t i = index_->Get(dic_entry); i < index_->Get((dic_entry + 1));
         ++i) {
      ret->push_back(position_->Get(i));
    }

    return ret;
  }

  /* *
   * @describe get lt groupkey part by value
   * @param[in] base value t( <t )
   *
   * @return result groupkey part, if no value return NULL
   * */
  std::shared_ptr<DGroupKey<T>> GetLTGroupkeyByValue(T value) {
    // if the value is less than the smallest value of the dic return the copy
    if (value > dic_->GetGreatestValue()) {
      return Copy();
    }

    if (value <= dic_->GetSmallestValue()) {
      return NULL;
    }

    // find the lower bound for value
    size_t value_id = dic_->FindLowerBoundForValue(value);
    size_t begin_entry = GetDicBeginEntry();

    return Split(begin_entry, value_id);
  }

  /* *
   * @describe get row id list, which value is less echo than target value
   * @param[in] base value t( <=t )
   *
   * @return result id list, if no value, return NULL
   * */
  std::shared_ptr<std::vector<uint64_t>> GetLERowIdListByValue(T value) {
    std::shared_ptr<std::vector<uint64_t>> result = NULL;

    if (value >= dic_->GetGreatestValue()) {
      result = std::make_shared<std::vector<uint64_t>>();
      for (size_t i = 0; i < position_->rows(); ++i) {
        result->push_back(position_->Get(i));
      }
    }

    if (value < dic_->GetSmallestValue()) {
      return result;
    }

    // find the upper bound for value
    size_t value_id = dic_->FindUpperBoundForValue(value);

    std::vector<T> value_list;
    for (size_t i = 0; i < value_id; ++i) {
      value_list.push_back(dic_->GetValueForValueId(i));
    }

    result = GetRowIdListByValueList(value_list);

    return result;
  }

  /* *
   * @describe get le groupkey part by value
   * @param[in] base value t( <=t )
   *
   * @return result groupkey part, if no value return NULL
   * */
  std::shared_ptr<DGroupKey<T>> GetLEGroupkeyByValue(T value) {
    // if the value is less than the smallest value of the dic return the copy
    if (value >= dic_->GetGreatestValue()) {
      return Copy();
    }

    if (value < dic_->GetSmallestValue()) {
      return NULL;
    }

    // find the lower bound for value
    size_t value_id = dic_->FindUpperBoundForValue(value);
    size_t begin_entry = GetDicBeginEntry();

    return Split(begin_entry, value_id);
  }

  std::shared_ptr<OrderedDictionary<T>> dic(void) const { return dic_; }

  std::shared_ptr<BitCompressedVector<uint64_t>> index() const {
    return index_;
  }

  std::shared_ptr<BitCompressedVector<uint64_t>> position() const {
    return position_;
  }

  /* *
   * @describe doing the groupkey update operation.
   *           the complexity of time is O(n) for update
   * @param[in] delta_data the delta data of the groupkey
   * @param[out] delta_x_vector the x_vecotr of delta dic
   *
   * @return the x vector, which produced during the process of update
   * */
  std::shared_ptr<std::vector<int64_t>> Update(
      std::shared_ptr<AbstractIndex> delta_data,
      std::shared_ptr<std::vector<int64_t>> &delta_x_vector) {
    std::shared_ptr<DGroupKey<T>> delta_ptr =
        std::dynamic_pointer_cast<DGroupKey<T>>(delta_data);

    // when dealing with the ordered dictionary with begin entry, we should be
    // careful to handle
    // this.
    size_t d_index = delta_ptr->dic()->entry_begin();
    size_t m_index = dic_->entry_begin();
    size_t m_index_offset(0), d_index_offset(0);
    size_t n = 0, count = 0, c = 0;
    std::vector<T> new_dic(dic_->Size() + delta_ptr->dic_->Size());

    std::shared_ptr<std::vector<int64_t>> x_vector =
        std::make_shared<std::vector<int64_t>>(dic_->Size());

    if (delta_x_vector == NULL) {
      delta_x_vector =
          std::make_shared<std::vector<int64_t>>(delta_ptr->dic_->Size());
    }

    std::vector<size_t> new_index_offset(dic_->Size() +
                                         delta_ptr->dic_->Size() + 1);
    std::vector<size_t> new_position(position_->rows() +
                                     delta_ptr->position_->rows());

    // using the delta groupkey bit_width, beacause the delta data's row id is
    // always bigger
    // than main data

    while ((d_index !=
            (delta_ptr->dic_->Size() + delta_ptr->dic_->entry_begin())) ||
           (m_index != (dic_->Size() + dic_->entry_begin()))) {
      bool main_dic_lt_delta = IsMainDicValLTDelta(delta_ptr, m_index, d_index);
      bool main_dic_gt_delta = IsMainDicValGTDelta(delta_ptr, m_index, d_index);
      new_index_offset[n] = c;

      if (main_dic_lt_delta) {
        try {
          new_dic[n] = dic_->GetValueForValueId(m_index);
        } catch (...) {
          LOG(ERROR) << "Invalid m_index: " << m_index
                     << " dic size: " << dic_->Size();
          return NULL;
        }

        (*x_vector)[m_index_offset] = n - m_index_offset;
        count = (*index_)[m_index_offset + 1] - (*index_)[m_index_offset];

        for (size_t i = 0; i < count; ++i) {
          new_position[c + i] = (*position_)[(*index_)[m_index_offset] + i];
        }

        c += count;
        m_index++;
        m_index_offset++;
      }

      if (main_dic_gt_delta) {
        try {
          new_dic[n] = delta_ptr->dic_->GetValueForValueId(d_index);
        } catch (...) {
          LOG(ERROR) << "Invalid d_index: " << d_index
                     << " dic size: " << dic_->Size();
          return NULL;
        }

        (*delta_x_vector)[d_index_offset] = n - d_index_offset;
        count = (*(delta_ptr->index_))[d_index_offset + 1] -
                (*(delta_ptr->index_))[d_index_offset];
        for (size_t i = 0; i < count; ++i) {
          new_position[c + i] =
              (*(delta_ptr
                     ->position_))[(*(delta_ptr->index_))[d_index_offset] + i];
        }

        c += count;
        d_index++;
        d_index_offset++;
      }
      n++;
    }

    new_index_offset[n] = c;

    //set max bit width;
    size_t max_index_value = *( std::max_element(new_index_offset.begin(), new_index_offset.end()));
    size_t max_position_value = *( std::max_element( new_position.begin(), new_position.end() ) );

    int bit_width_index = log2( max_index_value ) + 1;
    int bit_width_position = log2( max_position_value ) + 1;

    std::shared_ptr<OrderedDictionary<T>> main_dic =
        std::make_shared<OrderedDictionary<T>>(n);

    std::shared_ptr<BitCompressedVector<uint64_t>> main_index =
        std::make_shared<BitCompressedVector<uint64_t>>(n + 1, bit_width_index );

    std::shared_ptr<BitCompressedVector<uint64_t>> main_position =
        std::make_shared<BitCompressedVector<uint64_t>>(new_position.size(),
                                                        bit_width_position );

    for (size_t i = 0; i < n; ++i) {
      main_dic->AddValue(new_dic[i]);
      main_index->Set(i, new_index_offset[i]);
    }

    main_index->Set(n, new_index_offset[n]);

    for (size_t i = 0; i < new_position.size(); ++i) {
      main_position->Set(i, new_position[i]);
    }

    dic_ = main_dic;
    index_ = main_index;
    position_ = main_position;

    return x_vector;
  }

  uint64_t rows() { return position_->rows(); }

  void WriteLock() {}
  void UnLock() {}

  void Print() {
    dic_->Print();

    std::cout << "index: ";
    for (size_t i = 0; i < index_->rows(); ++i) {
      std::cout << (*index_)[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "position: ";
    for (size_t i = 0; i < position_->rows(); ++i) {
      std::cout << (*position_)[i] << " ";
    }
    std::cout << std::endl;
  }

  uint64_t Rows() { return position_->rows(); }

  uint64_t DictRows() { return dic_->Size(); }

  // statistic the total bytes of the DGroupkey
  uint64_t Statistic() {
    LOG(INFO) << "----dic size:" << dic_->Size() << " rows" << std::endl
              << "----index offset size:" << (index_->size()) << "bytes"
              << std::endl
              << "----index position size:" << (position_->size()) << "bytes"
              << std::endl;

    return (dic_->Statistic() + index_->size() + position_->size());
  }

 private:
  // this two function is used to deal with the update stream;
  inline bool IsMainDicValLTDelta(std::shared_ptr<DGroupKey<T>> delta_data,
                                  int m, int d) {
    if (d >= (delta_data->dic_->Size() + delta_data->dic_->entry_begin()))
      return true;

    // the main dic is
    if (m >= (dic_->Size() + dic_->entry_begin())) return false;

    T main_value = dic_->GetValueForValueId(m);
    T delta_value = delta_data->dic_->GetValueForValueId(d);

    return (main_value <= delta_value);
  }

  inline bool IsMainDicValGTDelta(std::shared_ptr<DGroupKey<T>> delta_data,
                                  int m, int d) {
    if (m >= (dic_->Size() + dic_->entry_begin())) return true;

    if (d >= (delta_data->dic_->Size() + delta_data->dic_->entry_begin()))
      return false;

    T main_value = dic_->GetValueForValueId(m);
    T delta_value = delta_data->dic_->GetValueForValueId(d);

    return (main_value >= delta_value);
  }

 public:
  /* *
   * @describe serialize the groupkey to string.
   * @param[out] std::string seriliaze STL string
   * */
  void SerializeToString(std::string &result) {
    std::string dic_serialize_string;
    std::string index_serialize_string;
    std::string position_serialize_string;

    dic_->SerializeToString(dic_serialize_string);
    index_->SerializeToString(index_serialize_string);
    position_->SerializeToString(position_serialize_string);

    size_t dic_str_length = dic_serialize_string.length();
    size_t index_str_length = index_serialize_string.length();
    size_t position_str_length = position_serialize_string.length();

    result.append((char *)(&dic_str_length), sizeof(size_t));
    result.append(dic_serialize_string);

    result.append((char *)(&index_str_length), sizeof(size_t));
    result.append(index_serialize_string);

    result.append((char *)(&position_str_length), sizeof(size_t));
    result.append(position_serialize_string);
  }

  /* *
   * @describe deserialize from the string.
   * @param[in] deserialize string
   * */
  void DeserializeFromString(std::string &data) {
    index_ = std::make_shared<BitCompressedVector<uint64_t>>();
    position_ = std::make_shared<BitCompressedVector<uint64_t>>();

    size_t dic_str_length, index_str_length, position_str_length;

    const char *cursor = data.data();

    memcpy(&dic_str_length, cursor, sizeof(size_t));
    cursor += sizeof(size_t);
    std::string dic_str(cursor, dic_str_length);
    cursor += dic_str_length;

    memcpy(&index_str_length, cursor, sizeof(size_t));
    cursor += sizeof(size_t);
    std::string index_str(cursor, index_str_length);
    cursor += index_str_length;

    memcpy(&position_str_length, cursor, sizeof(size_t));
    cursor += sizeof(size_t);
    std::string position_str(cursor, position_str_length);
    cursor += position_str_length;

    // doing the deserialize about the three vector
    dic_->DeserializeFromString(dic_str);
    index_->DeserializeFromString(index_str);
    position_->DeserializeFromString(position_str);
  }

  std::shared_ptr<DGroupKey<T>> Copy() {
    std::shared_ptr<OrderedDictionary<T>> copy_dic =
        std::dynamic_pointer_cast<OrderedDictionary<T>>(dic_->Copy());

    std::shared_ptr<BitCompressedVector<uint64_t>> copy_index =
        std::dynamic_pointer_cast<BitCompressedVector<uint64_t>>(
            index_->Copy());

    std::shared_ptr<BitCompressedVector<uint64_t>> copy_position =
        std::dynamic_pointer_cast<BitCompressedVector<uint64_t>>(
            position_->Copy());

    std::shared_ptr<DGroupKey<T>> ret =
        std::make_shared<DGroupKey<T>>(copy_dic, copy_index, copy_position);

    return ret;
  }

  DataType GetColumnType() {
    if (typeid(int) == typeid(T)) {
      return IntegerType;
    } else if (typeid(double) == typeid(T)) {
      return DoubleType;
    } else if (typeid(std::string) == typeid(T)) {
      return StringType;
    }
  }

  // get and set dictionary begin entry
  void SetDicBeginEntry(uint64_t begin_entry) {
    dic_->set_entry_begin(begin_entry);
  }

  uint64_t GetDicBeginEntry() const { return dic_->entry_begin(); }

  std::shared_ptr<DGroupKey<T>> get_shared_ptr() { return shared_from_this(); }

 private:
  // the dictionary
  std::shared_ptr<OrderedDictionary<T>> dic_;
  // the index offset
  std::shared_ptr<BitCompressedVector<uint64_t>> index_;
  // position
  std::shared_ptr<BitCompressedVector<uint64_t>> position_;
};

#endif  // DGROUPKEY_H_
