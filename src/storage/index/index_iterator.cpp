/**
 * index_iterator.cpp
 */
#include <cassert>

#include "storage/index/index_iterator.h"

namespace bustub {

/*
 * NOTE: you can change the destructor/constructor method here
 * set your own input parameters
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE::IndexIterator() = default;

INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE::IndexIterator(page_id_t page_id,BufferPoolManager *bpm,int idx){

  bpm_=bpm;
  pid=page_id;
  index=idx;
  if(page_id!=INVALID_PAGE_ID)node=GetNode(pid);
  if(node!= nullptr&&node->GetSize()==0)pid=INVALID_PAGE_ID;

}

INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE::~IndexIterator() {


  if(pid!=INVALID_PAGE_ID){

    bpm_->UnpinPage(pid, false);
  }

}// NOLINT

INDEX_TEMPLATE_ARGUMENTS
auto INDEXITERATOR_TYPE::IsEnd() -> bool {

  if(pid==INVALID_PAGE_ID)return true;

  return false;

}

INDEX_TEMPLATE_ARGUMENTS
auto INDEXITERATOR_TYPE::operator*() -> const MappingType & {

  //auto pair=node->GetEntry(index);
  //std::cout<<"iter:"<<pid<<" "<<index<<" "<<pair.first.ToString()<<std::endl;
  //LogTree("Insert","key:",key.ToString());
  return node->GetEntry(index);

}

INDEX_TEMPLATE_ARGUMENTS
auto INDEXITERATOR_TYPE::operator++() -> INDEXITERATOR_TYPE & {

  index++;

  //auto pair=node->GetEntry(index);
  //std::cout<<"iter:"<<pid<<" "<<index<<" " <<std::endl;

  if(index>=node->GetSize()){

      page_id_t  next=node->GetNextPageId();
      bpm_->UnpinPage(pid, false);

      pid=next;
      index=0;

      if(pid!=INVALID_PAGE_ID){

        node=GetNode(pid);
      }

      return *this;
  }

  return *this;
}

//self defined

INDEX_TEMPLATE_ARGUMENTS
auto INDEXITERATOR_TYPE::GetNode(page_id_t page_id) -> B_PLUS_TREE_LEAF_PAGE_TYPE * {

  auto page=bpm_->FetchPage(page_id);
  //page->RLatch();

  auto n=  reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(page->GetData());

  int size=n->GetSize();

  std::string s="";
  for(int i=0;i<size;i++){

    s=s+std::to_string( n->GetEntry(i).first.ToString())+" ";

  }

  //std::cout<<"node:"<<page_id<<" "<<"size:"<<size<<std::endl;
  //std::cout<<"elements::"<<s<<std::endl;


  return n;



}

template class IndexIterator<GenericKey<4>, RID, GenericComparator<4>>;

template class IndexIterator<GenericKey<8>, RID, GenericComparator<8>>;

template class IndexIterator<GenericKey<16>, RID, GenericComparator<16>>;

template class IndexIterator<GenericKey<32>, RID, GenericComparator<32>>;

template class IndexIterator<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
