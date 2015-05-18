/* Copyright 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IOTJS_UTIL_H
#define IOTJS_UTIL_H


#include <assert.h>


namespace iotjs {


char* ReadFile(const char* path);


char* AllocCharBuffer(size_t size);
void ReleaseCharBuffer(char* buff);


void PrintBacktrace();


template<class T>
struct LinkedListItem {
  LinkedListItem<T>(LinkedListItem* p, LinkedListItem* n, T d)
    : prev(p), next(n), data(d) {}

  LinkedListItem<T>* prev;
  LinkedListItem<T>* next;
  T data;
};


template<class T>
class LinkedListIterator {
public:
  LinkedListIterator<T>(LinkedListItem<T>* item)
    : _curr(item) {}

  bool Next() {
    assert(_curr != NULL);
    _curr = _curr->next;
    return _curr != NULL;
  }

private:
  LinkedListItem<T>* _curr;
};


template<class T>
class LinkedList {
public:
  LinkedList()
    : _head(NULL)
    , _tail(NULL)
    , _size(0) {}

  ~LinkedList() { Clear(); }

  inline size_t size() { return _size; }

  inline bool IsEmpty() { return _size == 0; }

  inline LinkedListItem<T>* head() { return _head; }
  inline LinkedListItem<T>* tail() { return _tail; }

  LinkedListItem<T>* InsertHead(T data) {
    LinkedListItem<T>* new_item = new LinkedListItem<T>(NULL, _head, data);
    if (IsEmpty()) {
      _head = _tail = new_item;
    } else {
      _head->prev= new_item;
      _head = new_item;
    }
    _size += 1;
    return new_item;
  }

  LinkedListItem<T>* InsertTail(T data) {
    LinkedListItem<T>* new_item = new LinkedListItem<T>(_tail, NULL, data);
    if (IsEmpty()) {
      _head = _tail = new_item;
    } else {
      _tail->next = new_item;
      _tail = new_item;
    }
    _size += 1;
    return new_item;
  }

  void RemoveHead() {
    assert(!IsEmpty());
    assert(_head != NULL);
    LinkedListItem<T>* old_head = _head;
    _size -= 1;
    if (_size == 0) {
      _head = _tail = NULL;
    } else {
      _head = _head->next;
      _head->prev = NULL;
    }
    delete old_head;
  }

  void RemoveTail() {
    assert(!IsEmpty());
    assert(_tail != NULL);
    LinkedListItem<T>* old_tail = _tail;
    _size -= 1;
    if (_size == 0) {
      _head = _tail = NULL;
    } else {
      _tail = _tail->prev;
      _tail->next = NULL;
    }
    delete old_tail;
  }

  void RemoveItem(LinkedListItem<T>* item) {
    assert(!IsEmpty());
    if (item->prev != NULL) {
      item->prev->next = item->next;
    } else {
      _head = item->next;
    }
    if (item->next != NULL) {
      item->next->prev = item->prev;
    } else {
      _tail = item->prev;
    }
    _size -= 1;
  }

  void Clear() {
    while (!IsEmpty()) {
      RemoveHead();
    }
    assert(IsEmpty());
    assert(head() == NULL);
    assert(tail() == NULL);
  }

private:
  LinkedListItem<T>* _head;
  LinkedListItem<T>* _tail;
  size_t _size;
};


} // namespace iotjs


#endif /* IOTJS_UTIL_H */
