//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/page/b_plus_tree_internal_page.cpp
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <iostream>
#include <sstream>

#include "common/config.h"
#include "common/exception.h"
#include "storage/page/b_plus_tree_internal_page.h"

namespace bustub {
/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/
/*
 * Init method after creating a new internal page
 * Including set page type, set current size, and set max page size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Init(int max_size) {

  this->SetPageType(IndexPageType::INTERNAL_PAGE);
  this->SetSize(0);
  this->SetMaxSize(max_size);
}
/*
 * Helper method to get/set the key associated with input "index"(a.k.a
 * array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::KeyAt(int index) const -> KeyType {
  // replace with your own code
  KeyType key{array_[index].first};
  return key;
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::SetKeyAt(int index, const KeyType &key) {

  array_[index].first=key;
  //IncreaseSize(1);
}

/*
 * Helper method to get the value associated with input "index"(a.k.a array
 * offset)
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::ValueAt(int index) const -> ValueType {

  return array_[index].second;
}

//self-defined
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Insert(const KeyType &key, page_id_t &page_id) {

//  int index=0;
//  while(KeyComparator(key,array_[index].first)<0){
//
//
//  }
//
//  page_id_t* pid;
//  auto np=bpm->NewPageGuarded(pid);


}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::Search(const KeyType &key) -> int {


  return 0;
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::GetPointer(int index) -> page_id_t {




  return 0;
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::Split()->std::pair<KeyType,page_id_t> {


    return std::pair<KeyType,page_id_t>(array_[0].first, array_[0].second);
}

// auto BPlusTreeInternalPage::Temp() ->int{


//   return 0;
// }

// valuetype for internalNode should be page id_t
template class BPlusTreeInternalPage<GenericKey<4>, page_id_t, GenericComparator<4>>;
template class BPlusTreeInternalPage<GenericKey<8>, page_id_t, GenericComparator<8>>;
template class BPlusTreeInternalPage<GenericKey<16>, page_id_t, GenericComparator<16>>;
template class BPlusTreeInternalPage<GenericKey<32>, page_id_t, GenericComparator<32>>;
template class BPlusTreeInternalPage<GenericKey<64>, page_id_t, GenericComparator<64>>;


//===================================
// template class BPlusTreeInternalPage<GenericKey<4>, RID, GenericComparator<4>>;

// template class BPlusTreeInternalPage<GenericKey<8>, RID, GenericComparator<8>>;

// template class BPlusTreeInternalPage<GenericKey<16>, RID, GenericComparator<16>>;

// template class BPlusTreeInternalPage<GenericKey<32>, RID, GenericComparator<32>>;

// template class BPlusTreeInternalPage<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
