/*
 * =================================================================
 *
 *            Filename:    AbstractDictionary.h
 *
 *         Description:    the abstract dictionary
 *
 *             Version:    v1.0
 *             Created:    2015-10-09 09:47
 *           Reversion:    none
 *            Compiler:    g++
 *
 *              Author:    wangjin, 836792834@qq.com
 *
 * ==================================================================
 */
#ifndef ABSTRACTDICTIONAY_H_
#define ABSTRACTDICTIONAY_H_
#include <sys/types.h>

class AbstractDictionary {
 public:
  AbstractDictionary() {}
  virtual ~AbstractDictionary() {}

  virtual size_t Size() const = 0;
};

#endif
