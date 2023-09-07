//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/include/index/index_iterator.h
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
/**
 * index_iterator.h
 * For range scan of b+ tree
 */
#pragma once
#include "storage/page/b_plus_tree_leaf_page.h"

namespace bustub {

#define INDEXITERATOR_TYPE IndexIterator<KeyType, ValueType, KeyComparator>

INDEX_TEMPLATE_ARGUMENTS
class IndexIterator {
 public:
  // you may define your own constructor based on your member variables
  IndexIterator();
  ~IndexIterator();  // NOLINT
  IndexIterator(page_id_t page_id,BufferPoolManager *bpm_,int idx);

  auto IsEnd() -> bool;

  auto operator*() -> const MappingType &;

  auto operator++() -> IndexIterator &;

  auto operator==(const IndexIterator &itr) const -> bool {

    if(itr.pid==INVALID_PAGE_ID&&pid==INVALID_PAGE_ID)return true;
    else if(itr.pid!=INVALID_PAGE_ID&&pid!=INVALID_PAGE_ID&&itr.pid==pid&&itr.index==index)return true;


    return false;
  }

  auto operator!=(const IndexIterator &itr) const -> bool {


        return !(itr==*this);

  }

  //self defined

  auto GetNode(page_id_t page_id) -> B_PLUS_TREE_LEAF_PAGE_TYPE *;

 private:
  // add your own private member variables here

  page_id_t pid;
  BufferPoolManager *bpm_;

  B_PLUS_TREE_LEAF_PAGE_TYPE * node= nullptr;
  int index=0;
};

}  // namespace bustub
