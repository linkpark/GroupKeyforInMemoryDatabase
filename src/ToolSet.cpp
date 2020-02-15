/*
 * =================================================================
 *
 *            Filename:    ToolSet.cpp
 *
 *         Description:    the speciliazed template of serialize function
 *
 *             Version:    v1.0
 *             Created:    2015-11-11 10:56
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */

#include "ToolSet.h"
template <>
void ToolSet::VectorSerializeToString<std::string>(
    std::vector<std::string> &origin_data, std::string &result) {
  size_t length = origin_data.size();
  size_t str_len(0);
  result.append((char *)(&length), sizeof(size_t));

  for (size_t i = 0; i < origin_data.size(); ++i) {
    str_len = origin_data[i].length();

    // append the length first, than append the data
    result.append((char *)(&str_len), sizeof(size_t));
    result.append(origin_data[i]);
  }
}

template <>
void ToolSet::VectorDeserializeFromString<std::string>(
    std::shared_ptr<std::vector<std::string> > &result, std::string &data) {
  if (!result) {
    result = std::make_shared<std::vector<std::string> >();
  }
  const char *cursor = data.data();

  size_t length(0);
  ::memcpy(&length, cursor, sizeof(size_t));
  cursor += sizeof(size_t);

  char *tmp_value;
  size_t str_len(0);

  for (size_t i = 0; i < length; ++i) {
    ::memcpy(&str_len, cursor, sizeof(size_t));
    cursor += sizeof(size_t);

    tmp_value = (char *)malloc(str_len);
    ::memcpy(tmp_value, cursor, str_len);
    cursor += str_len;

    std::string tmp(tmp_value, str_len);
    result->push_back(tmp);

    free(tmp_value);
    tmp_value = NULL;
  }
}
