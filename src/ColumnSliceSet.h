/*
 * =================================================================
 *
 *            Filename:    ColumnSliceSet.h
 *
 *         Description:    Column slice set, manange the sets of slice of one
 *                      column
 *
 *             Version:    v1.0
 *             Created:    2015-12-08 16:05
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */
#ifndef COLUMNSLICESET_H_
#define COLUMNSLICESET_H_
#include "BaseSliceSet.h"
#include "DGroupKey.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>

template <class T>
class ColumnSliceSet : public BaseSliceSet {
  typedef std::unordered_map<uint64_t, std::shared_ptr<AbstractIndex> >
      ColumnSliceMapType;
  typedef std::unordered_map<
      uint64_t, std::shared_ptr<std::unordered_map<uint64_t, uint64_t> > >
      ColumnSliceFourthVectorType;
  typedef typename std::unordered_map<uint64_t,
                                      std::shared_ptr<AbstractIndex> >::iterator
      ColumnSliceMapIteratorType;
  typedef std::unordered_map<uint64_t, std::shared_ptr<std::string> >
      ColumnSliceSerializeMapType;
  typedef typename std::unordered_map<uint64_t,
                                      std::shared_ptr<std::string> >::iterator
      ColumnSerializeSliceMapIteratorType;

 public:
  ColumnSliceSet() {}
  virtual ~ColumnSliceSet() {}

 public:
  // add one slice to the map
  void AddOneSlice(std::shared_ptr<AbstractIndex> &slice_ptr,
                   uint64_t slice_no) {
    if (slice_ptr == NULL) throw std::invalid_argument("slice ptr is NULL!");

    std::shared_ptr<DGroupKey<T> > groupkey_slice =
        std::dynamic_pointer_cast<DGroupKey<T> >(slice_ptr);

    if (groupkey_slice == NULL)
      throw std::invalid_argument("slice type is not match");

    // insert to map
    slice_set_.insert({slice_no, slice_ptr});
  }

  void GenerateForthVector(uint64_t slice_no) {
    std::shared_ptr<DGroupKey<T> > groupkey_index =
        std::dynamic_pointer_cast<DGroupKey<T> >(FindSliceBySliceNo(slice_no));

    GenerateForthVector(groupkey_index, slice_no);
  }

  void GenerateForthVector(std::shared_ptr<DGroupKey<T> > &groupkey_slice,
                           uint64_t slice_no) {
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
    // Insert to map
    if (fourth_vector_set_.find(slice_no) == fourth_vector_set_.end()) {
      fourth_vector_set_.insert({slice_no, fourth_vector_ptr});
    } else {
      fourth_vector_set_[slice_no] = fourth_vector_ptr;
    }

    LOG(INFO) << "Slice " << slice_no << "'s fourth vector generated, size: "
              << fourth_vector_ptr->size();
  }

  /* *
   * @describe add serialized data to the CS
   * @param[in] serialized_data the GroupKey serialized data
   * @param[in] slice_no the slice number of one Column
   * */
  void AddOneSlice(std::shared_ptr<std::string> &serialized_data,
                   uint64_t slice_no) {
    if (serialized_data == NULL)
      throw std::invalid_argument(
          "In ColumnSliceSet: \
                    AddOneSlice empty pointer of serialized data!");

    slice_serialize_set_.insert({slice_no, serialized_data});
  }

  std::shared_ptr<AbstractIndex> FindSliceBySliceNo(uint64_t slice_no) {
    ColumnSliceMapIteratorType it = slice_set_.find(slice_no);

    if (it == slice_set_.end()) return NULL;

    return it->second;
  }

  std::shared_ptr<std::string> FindSerializedSliceBySliceNo(uint64_t slice_no) {
    ColumnSerializeSliceMapIteratorType it =
        slice_serialize_set_.find(slice_no);
    if (it == slice_serialize_set_.end()) return NULL;

    return it->second;
  }

  std::shared_ptr<std::vector<int64_t> > Update(
      std::shared_ptr<AbstractIndex> &delta_index,
      std::shared_ptr<std::vector<int64_t> > &delta_x_vector, uint64_t slice_no,
      void *new_lower_bound, void *new_upper_bound, uint64_t &new_dic_size,
      uint64_t &dic_size_expan) {
    std::shared_ptr<DGroupKey<T> > groupkey_index =
        std::dynamic_pointer_cast<DGroupKey<T> >(delta_index);

    std::shared_ptr<AbstractIndex> base_column_slice_ptr =
        FindSliceBySliceNo(slice_no);

    if (base_column_slice_ptr == NULL)
      throw std::invalid_argument("can't find the slice by lower_bound");

    std::shared_ptr<DGroupKey<T> > column_slice_ptr =
        std::dynamic_pointer_cast<DGroupKey<T> >(base_column_slice_ptr);

    if (column_slice_ptr == NULL)
      throw std::invalid_argument("type is not match!");

    uint64_t old_dic_size = column_slice_ptr->dic()->Size();

    std::shared_ptr<std::vector<int64_t> > ret =
        column_slice_ptr->Update(delta_index, delta_x_vector);

    T *new_lower_bound_value = static_cast<T *>(new_lower_bound);
    T *new_upper_bound_value = static_cast<T *>(new_upper_bound);

    // set new lower bound and upper bound
    *new_lower_bound_value = column_slice_ptr->dic()->GetSmallestValue();
    *new_upper_bound_value = column_slice_ptr->dic()->GetGreatestValue();
    // set new dic number;
    new_dic_size = column_slice_ptr->dic()->Size();
    dic_size_expan = new_dic_size - old_dic_size;

    return ret;
  }

  std::shared_ptr<std::vector<int64_t> > Update(
      std::shared_ptr<AbstractIndex> &delta_index,
      std::shared_ptr<std::vector<int64_t> > &delta_x_vector, uint64_t slice_no,
      uint64_t &new_dic_size, uint64_t &dic_size_expan) {
    std::shared_ptr<DGroupKey<T> > groupkey_index =
        std::dynamic_pointer_cast<DGroupKey<T> >(delta_index);

    std::shared_ptr<AbstractIndex> base_column_slice_ptr =
        FindSliceBySliceNo(slice_no);

    if (base_column_slice_ptr == NULL)
      throw std::invalid_argument("can't find the slice by lower_bound");

    std::shared_ptr<DGroupKey<T> > column_slice_ptr =
        std::dynamic_pointer_cast<DGroupKey<T> >(base_column_slice_ptr);

    uint64_t old_dic_size = column_slice_ptr->dic()->Size();

    if (column_slice_ptr == NULL)
      throw std::invalid_argument("type is not match!");

    std::shared_ptr<std::vector<int64_t> > ret =
        column_slice_ptr->Update(delta_index, delta_x_vector);

    // if the lower bound changed, we should change the map
    T new_lower_bound = column_slice_ptr->dic()->GetSmallestValue();

    new_dic_size = column_slice_ptr->dic()->Size();
    dic_size_expan = new_dic_size - old_dic_size;

    return ret;
  }

  uint64_t Rows() {
    uint64_t count = 0;
    for (auto ix : slice_set_) {
      count += ix.second->Rows();
    }

    return count;
  }

  void Print() {
    for (auto ix : slice_set_) {
      std::cout << "begin slice: " << ix.first << std::endl;
      ix.second->Print();
    }
  }

  void PrintSlice() {
    for (auto ix : slice_set_) {
      std::cout << "slice: " << ix.first << std::endl;
    }
  }

  uint64_t Statistic() {
    uint64_t sum(0);

    for (auto ix : slice_set_) {
      LOG(INFO) << " slice begin: " << ix.first;
      sum += ix.second->Statistic();
    }

    return sum;
  }

  bool IsSliceNoExist(uint64_t slice_no) {
    ColumnSliceMapIteratorType it = slice_set_.find(slice_no);

    if (it == slice_set_.end()) return false;

    return true;
  }

  // Functions of Fourth Vector
  std::shared_ptr<std::unordered_map<uint64_t, uint64_t> >
  FindSliceFourthVectorBySliceNo(uint64_t slice_no) {
    auto fourth_it = fourth_vector_set_.find(slice_no);
    if (fourth_it == fourth_vector_set_.end()) {
      LOG(ERROR) << "Can't find Slice " << slice_no << "'s Fourth Vector";
      return NULL;
    } else
      return fourth_it->second;
  }

 private:
  // use the dic lower bound as is index
  ColumnSliceMapType slice_set_;
  ColumnSliceFourthVectorType fourth_vector_set_;
  ColumnSliceSerializeMapType slice_serialize_set_;
};

#endif
