/*
 * Copyright 2019 - 2020 Elie Michel
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** \file
 * \ingroup openmesheffect
 *
 */

#ifndef __MFX_COLLECTION_H__
#define __MFX_COLLECTION_H__

#include <vector>
#include <stdexcept>

namespace OpenMfx {

/**
 * Common mechanism for propoerty, input and attribute sets
 * Element type must implement following methods:
 *     void setIndex(const Index& index);
 *     Index index() const;
 * and it is advised to define the Index type:
 *     using Index = ...;
 * They must be movable but not necessarily copyable
 */
template<typename T, typename Index = T::Index> class Collection {
 public:
  Collection()
  {
  }
  Collection(const Collection<T, Index> &) = delete;
  Collection<T, Index> &operator=(const Collection<T, Index> &) = delete;
  Collection(Collection<T, Index> &&) = default;
  Collection<T, Index> &operator=(Collection<T, Index> &&) = default;

  int find(const Index &index) const
  {
    int i = -1;
    for (const T &item : m_items) {
      ++i;
      if (item.index() == index) {
        return i;
      }
    }
    return -1;
  }

  void append(int count)
  {
    int oldCount = m_items.size();
    m_items.resize(oldCount + count);
    for (int i = 0; i < count; ++i) {
      onNewItem(m_items[oldCount + i]);
    }
  }

  int ensure(const Index &index)
  {
    int i = find(index);
    if (i > -1) {
      return i;
    }
    append(1);
    m_items.back().setIndex(index);
    return m_items.size() - 1;
  }

  void remove(int index)
  {
    m_items.erase(m_items.begin() + index);
  }

  virtual void deep_copy_from(const Collection<T, Index> &other)
  {
    m_items.resize(other.count());
    for (int i = 0; i < count(); ++i) {
      m_items[i].deep_copy_from(other.m_items[i]);
    }
  }

  int count() const
  {
    return m_items.size();
  }

  T &operator[](int i)
  {
    return m_items[i];
  }

  T &operator[](const Index &index)
  {
    int i = ensure(index);
    return m_items[i];
  }

  const T &operator[](int i) const
  {
    return m_items[i];
  }

  const T &operator[](const Index &index) const
  {
    int i = find(index);
    if (i == -1)
      throw std::invalid_argument("Collection has no item at this index");
    return m_items[i];
  }

  template<typename = typename std::enable_if<std::is_same<T, std::string>::value>::type>
  T &operator[](const char *charIndex)
  {
    (*this)[Index(charIndex)];
  }

  template<typename = typename std::enable_if<std::is_same<T, std::string>::value>::type>
  const T &operator[](const char *charIndex) const
  {
    (*this)[Index(charIndex)];
  }

  virtual void onNewItem(T &item)
  {
  }

  void clear()
  {
    m_items.clear();
  }

 private:
  std::vector<T> m_items;
};

}  // namespace OpenMfx

#endif // __MFX_COLLECTION_H__
