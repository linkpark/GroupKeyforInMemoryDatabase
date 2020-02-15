/*
 * =================================================================
 *
 *            Filename:    BaseSliceSet.h
 *
 *         Description:    the base class of ColumnSlic
 *
 *             Version:    v1.0
 *             Created:    2015-12-08 16:58
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */

#ifndef BASESLICESET_H_
#define BASESLICESET_H_
#include <memory>
#include <unordered_map>
#include <vector>
#include "AbstractIndex.h"

class BaseSliceSet {
 public:
  BaseSliceSet() {}
  virtual ~BaseSliceSet() {}

 public:
  virtual void AddOneSlice(std::shared_ptr<AbstractIndex> &slice_ptr,
                           uint64_t slice_no) = 0;

  virtual void Print() = 0;

  virtual void PrintSlice() = 0;

  virtual bool IsSliceNoExist(uint64_t slice_no) = 0;

  virtual std::shared_ptr<std::vector<int64_t> > Update(
      std::shared_ptr<AbstractIndex> &delta_index,
      std::shared_ptr<std::vector<int64_t> > &delta_x_vector, uint64_t slice_no,
      void *new_dic_lower_bound, void *new_dic_upper_bound,
      uint64_t &new_dic_size, uint64_t &dic_size_expan) = 0;

  virtual std::shared_ptr<std::vector<int64_t> > Update(
      std::shared_ptr<AbstractIndex> &delta_index,
      std::shared_ptr<std::vector<int64_t> > &delta_x_vector, uint64_t slice_no,
      uint64_t &new_dic_size, uint64_t &dic_size_expan) = 0;

  virtual uint64_t Rows() = 0;

  virtual uint64_t Statistic() = 0;

  virtual std::shared_ptr<AbstractIndex> FindSliceBySliceNo(
      uint64_t slice_no) = 0;

  // return the forth vecotr
  virtual std::shared_ptr<std::unordered_map<uint64_t, uint64_t> >
  FindSliceFourthVectorBySliceNo(uint64_t slice_no) = 0;

  virtual void GenerateForthVector(uint64_t slice_no) = 0;
};

#endif
