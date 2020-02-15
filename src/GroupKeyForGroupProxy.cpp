/*
 * =================================================================
 *
 *            Filename:    GroupKeyForGroupProxy.cpp
 *
 *         Description:    the implements of GroupKeyForGroupProxy
 *
 *             Version:
 *             Created:    2016-04-20 17:30
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */
#include "GroupKeyForGroupProxy.h"
#include "ColumnMetaData.h"
#include "StorageTypes.h"

using namespace std::placeholders;

GroupKeyForGroupProxy::GroupKeyForGroupProxy() {
  RegisterOpFunc(GroupType::FUN_COUNT,
                 std::bind(&GroupKeyForGroupProxy::CountGroupFunc, this, _1, _2,
                           _3, _4, _5));

  RegisterOpFunc(GroupType::FUN_MAX,
                 std::bind(&GroupKeyForGroupProxy::MaxGroupFunc, this, _1, _2,
                           _3, _4, _5));

  RegisterOpFunc(GroupType::FUN_MIN,
                 std::bind(&GroupKeyForGroupProxy::MinGroupFunc, this, _1, _2,
                           _3, _4, _5));

  RegisterOpFunc(GroupType::FUN_SUM,
                 std::bind(&GroupKeyForGroupProxy::SumGroupFunc, this, _1, _2,
                           _3, _4, _5));
}


void GroupKeyForGroupProxy::MaxGroupFunc(int value_type, 
                    std::shared_ptr<AbstractIndex> column_data,
                    std::shared_ptr<ForthVectorType> fourth_vector,
                    std::vector<uint64_t> &row_id_list, void *result) {
  switch( value_type ) {
    case DataType::DoubleType:
      MaxGroupOp<double>( column_data, fourth_vector, row_id_list, result );
      break;
    case DataType::StringType:
      MaxGroupOp< std::string >( column_data, fourth_vector, row_id_list, result );
      break;
    case DataType::IntegerType:
      MaxGroupOp<uint64_t>( column_data, fourth_vector, row_id_list, result );
      break; 
  } 
}

void GroupKeyForGroupProxy::MinGroupFunc(int value_type, 
                    std::shared_ptr<AbstractIndex> column_data,
                    std::shared_ptr<ForthVectorType> fourth_vector,
                    std::vector<uint64_t> &row_id_list, void *result) {
  switch( value_type ) {
    case DataType::DoubleType:
      MinGroupOp<double>( column_data, fourth_vector, row_id_list, result );
      break;
    case DataType::StringType:
      MinGroupOp< std::string >( column_data, fourth_vector, row_id_list, result );
      break;
    case DataType::IntegerType:
      MinGroupOp<uint64_t>( column_data, fourth_vector, row_id_list, result );
      break; 
  } 
}

void GroupKeyForGroupProxy::CountGroupFunc(int value_type, 
                    std::shared_ptr<AbstractIndex> column_data,
                    std::shared_ptr<ForthVectorType> fourth_vector,
                    std::vector<uint64_t> &row_id_list, void *result) {
  switch( value_type ) {
    case DataType::DoubleType:
      CountGroupOp<double>( column_data, fourth_vector, row_id_list, result );
      break;
    case DataType::StringType:
      CountGroupOp< std::string >( column_data, fourth_vector, row_id_list, result );
      break;
    case DataType::IntegerType:
      CountGroupOp<uint64_t>( column_data, fourth_vector, row_id_list, result );
      break; 
  } 
}

void GroupKeyForGroupProxy::SumGroupFunc(int value_type, 
                    std::shared_ptr<AbstractIndex> column_data,
                    std::shared_ptr<ForthVectorType> fourth_vector,
                    std::vector<uint64_t> &row_id_list, void *result) {
  switch( value_type ) {
    case DataType::DoubleType:
      SumGroupOp<double>( column_data, fourth_vector, row_id_list, result );
      break;
    case DataType::StringType:
      SumGroupOp< std::string >( column_data, fourth_vector, row_id_list, result );
      break;
    case DataType::IntegerType:
      SumGroupOp<uint64_t>( column_data, fourth_vector, row_id_list, result );
      break; 
  } 
}

