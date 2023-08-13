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
#include <utility>

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
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Insert(int index, const KeyType &key,page_id_t &page_id,const KeyComparator &comparator) {

    //index++;
    // for (index = 0; index < GetSize(); ++index) {
    //     // if (comparator(KeyAt(index),key)  == 0) {
    //     //     // Key already exists, return false
    //     //     return false;
    //     // }
    //     if (comparator(KeyAt(index),key)>0) {
    //         break;
    //     }
    // }

    // Shift all elements after index to the right by 1
    for (int i = GetSize(); i > index; --i) {
        array_[i] = array_[i - 1];
    }

    // Insert new key-value pair
    //SetValueAtIndex(index, std::make_pair(key, value));
    array_[index]=std::make_pair(key, page_id);

    // Increase the size by 1
    IncreaseSize(1);

    //return true;


}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::Search(const KeyType &key,const KeyComparator &comparator) -> int {


    int left=1;int right=GetSize()-1;

    while(left<=right){

      int mid=(left+right)/2;

      
      if(comparator(array_[mid].first,key)<=0)left=mid+1;
      else if(comparator(array_[mid].first,key)>0)right=mid-1;

    }

    return right;
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::GetPointer(int index) -> page_id_t {




  return array_[index].second;
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::Split(BufferPoolManager *bpm)->std::pair<KeyType,page_id_t> {

  page_id_t pid;
  auto guard= bpm->NewPageGuarded(&pid);

  
  //根据page的大小以及key的大小，Max_Size>=3(Max_Size==2时不方便split)
  int cutPos=(GetSize())/2;

  

  auto page=reinterpret_cast<BPlusTreeInternalPage<KeyType,ValueType,KeyComparator> *>(guard.GetDataMut());
  page->Init(GetMaxSize());

  
  
  for(int i=cutPos+1;i<GetSize();i++){

    page->array_[i-(cutPos+1)+1]=array_[i];
  }

   page->array_[0]=std::make_pair(KeyType{1}, array_[cutPos].second);

  page->SetSize(GetSize()-cutPos);
  SetSize(cutPos);

   
  return std::pair<KeyType,page_id_t>(array_[cutPos].first,pid);
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::Split(BufferPoolManager *bpm,int flag)->std::pair<KeyType,page_id_t> {

  page_id_t pid;
  auto guard= bpm->NewPageGuarded(&pid);

  
  //根据page的大小以及key的大小，Max_Size>=3(Max_Size==2时不方便split)
  int cutPos=(GetSize())/2;

  /**
          0      1 
      (k1,p1) (k1,p1) 

          0      1       2
      (k1,p1) (k1,p1) (k1,p1)
         
  */
  if(flag==1)cutPos++;

  

  auto page=reinterpret_cast<BPlusTreeInternalPage<KeyType,ValueType,KeyComparator> *>(guard.GetDataMut());
  page->Init(GetMaxSize());

  
  
  for(int i=cutPos+1;i<GetSize();i++){

    page->array_[i-(cutPos+1)+1]=array_[i];
  }

   page->array_[0]=std::make_pair(KeyType{1}, array_[cutPos].second);

  page->SetSize(GetSize()-cutPos);
  SetSize(cutPos);

   
  return std::pair<KeyType,page_id_t>(array_[cutPos].first,pid);
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::SplitAndInsert(BufferPoolManager *bpm,int index, const KeyType &key, page_id_t &page_id,const KeyComparator &comparator)->std::pair<KeyType,page_id_t> {




    page_id_t pid;
    auto guard= bpm->NewPageGuarded(&pid);


    //根据page的大小以及key的大小，Max_Size>=3(Max_Size==2时不方便split)
    int cutPos=(GetSize())/2;



    auto page=reinterpret_cast<BPlusTreeInternalPage<KeyType,ValueType,KeyComparator> *>(guard.GetDataMut());
    page->Init(GetMaxSize());



    for(int i=cutPos+1;i<GetSize();i++){

        page->array_[i-(cutPos+1)+1]=array_[i];
    }

    page->array_[0]=std::make_pair(KeyType{1}, array_[cutPos].second);

    page->SetSize(GetSize()-cutPos);
    SetSize(cutPos);

    KeyType pivot= array_[cutPos].first;

    if(index<cutPos){

        Insert(index+1,key,page_id,comparator);
        //SetSize(GetSize()+1);
    }else{

        int pos=page->Search(key,comparator);

        page->Insert(pos+1,key,page_id,comparator);

        //page->SetSize(page->GetSize()+1);
    }



    if(GetSize()<=1){

        SetSize(GetSize()+1);



        for(int i=1;i<page->GetSize();i++){

            page->array_[i-1]=page->array_[i];
        }

        page->SetSize(page->GetSize()-1);


        pivot=page->array_[0].first;
        page->array_[0].first=KeyType{1};
    }


    return std::pair<KeyType,page_id_t>(pivot,pid);
}


INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Delete(int index, const KeyComparator &comparator){

    int size=GetSize();

    for(int i=index+1;i<size;i++){

            array_[i-1]=array_[i];
    }
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Move(B_PLUS_TREE_INTERNAL_PAGE_TYPE *p1,B_PLUS_TREE_INTERNAL_PAGE_TYPE *p2,const KeyType &key){




}

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
