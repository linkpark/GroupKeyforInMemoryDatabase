/*
 * =================================================================
 *
 *            Filename:    GroupKeyForGroupProxy.h
 *
 *         Description:    dealing with group operator
 *
 *             Version:    v1.0
 *             Created:    2016-04-20 15:54
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */
#ifndef GROUPKEYFORGROUPPROXY_H_
#define GROUPKEYFORGROUPPROXY_H_

#include "AbstractIndex.h"
#include "DGroupKey.h"

#include <algorithm>
#include <exception>
#include <functional>
#include <memory>
#include <unordered_map>
#include <numeric>

#include <glog/logging.h>

enum GroupType {
  FUN_COUNT = 2501,
  FUN_SUM,
  FUN_AVG,
  FUN_MAX,
  FUN_MIN,
  FUN_UPPER_BOUND
};

class GroupKeyForGroupProxy {
  typedef std::unordered_map<uint64_t, uint64_t> ForthVectorType;
  typedef std::function<void(int, std::shared_ptr<AbstractIndex>,
                             std::shared_ptr<ForthVectorType>,
                             std::vector<uint64_t> &, void *)>
      GroupFunc;

 public:
  GroupKeyForGroupProxy();
  virtual ~GroupKeyForGroupProxy() {}

  void CalculateGroupOp(int op_type, int value_type,
                        std::shared_ptr<AbstractIndex> column_data,
                        std::shared_ptr<ForthVectorType> fourth_vector,
                        std::vector<uint64_t> &row_id_list, void *result) {
    group_func_map_[op_type](value_type, column_data, fourth_vector,
                             row_id_list, result);
  }

  void RegisterOpFunc(int op_type, GroupFunc group_func) {
    group_func_map_.insert({op_type, group_func});
  }

  void MinGroupFunc(int value_type, std::shared_ptr<AbstractIndex> column_data,
                    std::shared_ptr<ForthVectorType> fourth_vector,
                    std::vector<uint64_t> &row_id_list, void *result);

  void MaxGroupFunc(int value_type, std::shared_ptr<AbstractIndex> column_data,
                    std::shared_ptr<ForthVectorType> fourth_vector,
                    std::vector<uint64_t> &row_id_list, void *result);

  void CountGroupFunc(int value_type,
                      std::shared_ptr<AbstractIndex> column_data,
                      std::shared_ptr<ForthVectorType> fourth_vector,
                      std::vector<uint64_t> &row_id_list, void *result);

  void SumGroupFunc(int value_type,
                      std::shared_ptr<AbstractIndex> column_data,
                      std::shared_ptr<ForthVectorType> fourth_vector,
                      std::vector<uint64_t> &row_id_list, void *result);

  template <class T>
  void MaxGroupOp(std::shared_ptr<AbstractIndex> column_data,
                  std::shared_ptr<ForthVectorType> fourth_vector,
                  std::vector<uint64_t> &row_id_list, void *result) {
    std::shared_ptr<DGroupKey<T> > column =
        std::dynamic_pointer_cast<DGroupKey<T> >(column_data);

    std::vector<T> value_list;
    for( auto ix: row_id_list ) {
      auto it = fourth_vector->find( ix );
      if( it != fourth_vector->end() ) {
        T value = column->dic()->GetValueForValueId( it->second );
        value_list.push_back( value );
      }
    }

    auto ix = std::max_element(value_list.begin(), value_list.end());

    T *value_result = static_cast< T* >( result );
    *value_result = *ix;
  }

  template <class T>
  void MinGroupOp(std::shared_ptr<AbstractIndex> column_data,
                  std::shared_ptr<ForthVectorType> fourth_vector,
                  std::vector<uint64_t> &row_id_list, void *result) {
    std::shared_ptr<DGroupKey<T> > column =
        std::dynamic_pointer_cast<DGroupKey<T> >(column_data);

    std::vector<T> value_list;
    for( auto ix: row_id_list ) {
      auto it = fourth_vector->find( ix );
      if( it != fourth_vector->end() ) {
        T value = column->dic()->GetValueForValueId( it->second );
        value_list.push_back( value );
      }
    }

    auto ix = std::min_element(value_list.begin(), value_list.end());

    T *value_result = static_cast< T* >( result );
    *value_result = *ix;
  }

  template <class T>
  void CountGroupOp(std::shared_ptr<AbstractIndex> column_data,
                    std::shared_ptr<ForthVectorType> fourth_vector,
                    std::vector<uint64_t> &row_id_list, void *result) {
    std::shared_ptr<DGroupKey<T> > column =
        std::dynamic_pointer_cast<DGroupKey<T> >(column_data);

    std::vector<T> value_list;
    for( auto ix: row_id_list ) {
      auto it = fourth_vector->find( ix );
      if( it != fourth_vector->end() ) {
        T value = column->dic()->GetValueForValueId( it->second );
        value_list.push_back( value );
      }
    }
 
    uint64_t *value_result = static_cast< uint64_t *>(result);
    *value_result = value_list.size(); 
  }

  template <class T>
  void SumGroupOp(std::shared_ptr<AbstractIndex> column_data,
                    std::shared_ptr<ForthVectorType> fourth_vector,
                    std::vector<uint64_t> &row_id_list, void *result) {
    std::shared_ptr<DGroupKey<T> > column =
        std::dynamic_pointer_cast<DGroupKey<T> >(column_data);

    std::vector<T> value_list;
    for( auto ix: row_id_list ) {
      auto it = fourth_vector->find( ix );
      if( it != fourth_vector->end() ) {
        T value = column->dic()->GetValueForValueId( it->second );
        value_list.push_back( value );
      }
    }

    T value_result = 
      std::accumulate( value_list.begin() + 1 , value_list.end(), value_list.front() );
    
    T *result_t = static_cast< T* >(result);
    *result_t = value_result;
  }

 private:
  std::unordered_map<int, GroupFunc> group_func_map_;
};

static GroupKeyForGroupProxy g_groupkey_group_proxy;

#endif

