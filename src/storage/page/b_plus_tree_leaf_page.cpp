//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/page/b_plus_tree_leaf_page.cpp
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <string.h>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <utility>

#include "common/config.h"
#include "common/exception.h"
#include "common/rid.h"
#include "storage/page/b_plus_tree_leaf_page.h"
#include "type/value.h"

namespace bustub {

/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/

/**
 * Init method after creating a new leaf page
 * Including set page type, set current size to zero, set next page id and set max size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::Init(int max_size) {

  this->SetPageType(IndexPageType::LEAF_PAGE);
  this->SetSize(0);
  this->SetMaxSize(max_size);

}

/**
 * Helper methods to set/get next page id
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::GetNextPageId() const -> page_id_t {

  return next_page_id_;
  //return INVALID_PAGE_ID;

}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::SetNextPageId(page_id_t next_page_id) {

  next_page_id_=next_page_id;
}

/*
 * Helper method to find and return the key associated with input "index"(a.k.a
 * array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::KeyAt(int index) const -> KeyType {
  // replace with your own code
  KeyType key{array_[index].first};
  return key;
}

//self-defined

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::Insert(const KeyType &key, const ValueType &value,const KeyComparator &comparator) ->bool{

    // if (GetSize() >= GetMaxSize()) {
    //     // Leaf node is full, need to split, which is not handled in this example
    //     return false;
    // }

    int index;
    for (index = 0; index < GetSize(); ++index) {
        if (comparator(KeyAt(index),key)  == 0) {
            // Key already exists, return false
            return false;
        }
        if (comparator(KeyAt(index),key)>0) {
            break;
        }
    }

    // Shift all elements after index to the right by 1
    for (int i = GetSize(); i > index; --i) {
        array_[i] = array_[i - 1];
    }

    // Insert new key-value pair
    //SetValueAtIndex(index, std::make_pair(key, value));
    array_[index]=std::make_pair(key, value);

    // Increase the size by 1
    IncreaseSize(1);

    return true;

    
    
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::Split(BufferPoolManager *bpm)->std::pair<KeyType,page_id_t>{

  page_id_t pid;
  auto guard= bpm->NewPageGuarded(&pid);

  
  int cutPos=(GetSize()-1)/2;

  

  auto page=reinterpret_cast<BPlusTreeLeafPage<KeyType,ValueType,KeyComparator> *>(guard.GetDataMut());
  page->Init();
  
  for(int i=cutPos+1;i<GetSize();i++){

    page->array_[i-(cutPos+1)]=array_[i];
  }

 

  page->SetSize(GetSize()-1-cutPos);
  SetSize(cutPos+1);

  next_page_id_=pid;

   
  return std::pair<KeyType,page_id_t>(page->array_[0].first,pid);
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::Search(const KeyType &key,const KeyComparator &comparator) -> int{


    
    int left=0;int right=GetSize()-1;

    while(left<=right){

      int mid=(left+right)/2;

      
      if(comparator(array_[mid].first,key)<0)left=mid+1;
      else if(comparator(array_[mid].first,key)>0)right=mid-1;
      else return mid;


    }

    return -1;
  
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::Get(int index) -> ValueType{


  return array_[index].second;
}


template class BPlusTreeLeafPage<GenericKey<4>, RID, GenericComparator<4>>;
template class BPlusTreeLeafPage<GenericKey<8>, RID, GenericComparator<8>>;
template class BPlusTreeLeafPage<GenericKey<16>, RID, GenericComparator<16>>;
template class BPlusTreeLeafPage<GenericKey<32>, RID, GenericComparator<32>>;
template class BPlusTreeLeafPage<GenericKey<64>, RID, GenericComparator<64>>;
}  // namespace bustub
