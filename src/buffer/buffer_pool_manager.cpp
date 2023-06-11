//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager.h"

#include "common/exception.h"
#include "common/macros.h"
#include "storage/page/page_guard.h"

namespace bustub {

BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager *disk_manager, size_t replacer_k,
                                     LogManager *log_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager), log_manager_(log_manager) {
  // TODO(students): remove this line after you have implemented the buffer pool manager
//  throw NotImplementedException(
//      "BufferPoolManager is not implemented yet. If you have finished implementing BPM, please remove the throw "
//      "exception line in `buffer_pool_manager.cpp`.");

  // we allocate a consecutive memory space for the buffer pool
  pages_ = new Page[pool_size_];
  replacer_ = std::make_unique<LRUKReplacer>(pool_size, replacer_k);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }

  LOG("BufferPoolManager","pool_size",pool_size,"replacer_k",replacer_k);
}

BufferPoolManager::~BufferPoolManager() { delete[] pages_; }

auto BufferPoolManager::NewPage(page_id_t *page_id) -> Page * {
  LOG("NewPage");
  frame_id_t fid;
  if(!free_list_.empty()){

    fid= free_list_.back();
    free_list_.pop_back();

  }else if(replacer_->Size()!=0){

    replacer_->Evict(&fid);

  }else return nullptr;

  auto pid=AllocatePage();
  LOG("NewPage",pid);
  *page_id=pid;
  if(pages_[fid].is_dirty_) FlushPage(pages_[fid].page_id_);

  page_table_.erase(pages_[fid].page_id_);//要及时从page table中erase掉
  Page* page=&pages_[fid];

  page->ResetMemory();
  page->page_id_=pid;
  page->is_dirty_=false;
  page->pin_count_=1;
  LOG("PagePin",page->page_id_,page->pin_count_);
  page_table_[pid]=fid;

  replacer_->SetEvictable(fid,false);

  replacer_->RecordAccess(fid);


  return page;


}

auto BufferPoolManager::FetchPage(page_id_t page_id, [[maybe_unused]] AccessType access_type) -> Page * {

  LOG("FetchPage",page_id);
  if(page_table_.find(page_id)!=page_table_.end()){

    Page& page=pages_[page_table_.at(page_id)];
    page.pin_count_++;// fetch要将pin+1

    replacer_->SetEvictable(page_table_.at(page_id),false);

    replacer_->RecordAccess(page_table_.at(page_id));

    LOG("PagePin",page.page_id_,page.pin_count_);
    return &page;
  }

  frame_id_t fid;
  if(!free_list_.empty()){

    fid= free_list_.back();
    free_list_.pop_back();

  }else if(replacer_->Size()!=0){

    replacer_->Evict(&fid);

  }else return nullptr;


  LOG("FetchPage replace",pages_[fid].page_id_);
  if(pages_[fid].is_dirty_) FlushPage(pages_[fid].page_id_);
  page_table_.erase(pages_[fid].page_id_);

  Page* page=&pages_[fid];

  page->ResetMemory();
  page->page_id_=page_id;
  page->is_dirty_=false;
  page->pin_count_=1;
  LOG("PagePin",page->page_id_,page->pin_count_);
  disk_manager_->ReadPage(page_id,page->data_);
  LOG("ReadPage",std::string(page->data_));
  page_table_[page_id]=fid;

  replacer_->SetEvictable(fid,false);

  replacer_->RecordAccess(fid);

  return page;
}

auto BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty, [[maybe_unused]] AccessType access_type) -> bool {
  LOG("UnpinPage",page_id,is_dirty);
  if(page_table_.find(page_id)==page_table_.end())return false;

  auto it=page_table_.find(page_id);
  auto fid=it->second;
  Page& page=pages_[fid];



  if(page.GetPinCount()==0)return false;

  page.pin_count_--;
  LOG("PagePin",page.page_id_,page.pin_count_);

  /**
   * 对于某一个page,任何时候只要dirty了,后面就一直是dirty状态,
   * 除非被flush,delete,或者重置成新的page.
   */
  if(is_dirty)page.is_dirty_=is_dirty;


  if(page.GetPinCount()==0){

    replacer_->SetEvictable(fid,true);
  }

  return true;
}

auto BufferPoolManager::FlushPage(page_id_t page_id) -> bool {
  LOG("FlushPage",page_id);
  if(page_id==INVALID_PAGE_ID)return false;
  if(page_table_.find(page_id)==page_table_.end())return false;

  auto fid=page_table_.at(page_id);
  Page& page=pages_[fid];

  disk_manager_->WritePage(page_id,page.GetData());
  page.is_dirty_= false;
  page.page_id_=INVALID_PAGE_ID;
  page_table_.erase(page_id);

  return true;


}

void BufferPoolManager::FlushAllPages() {

  LOG("FlushAllPages");
  for(auto it=page_table_.begin();it!=page_table_.end();it++){
    FlushPage(it->first);
  }

}

auto BufferPoolManager::DeletePage(page_id_t page_id) -> bool {
  LOG("DeletePage",page_id);
  if(page_table_.find(page_id)==page_table_.end())return true;

  if(pages_[page_table_.at(page_id)].GetPinCount()>0)return false;

  Page& page=pages_[page_table_.at(page_id)];
  free_list_.push_front(page_table_.at(page_id));//将frame重新放回free_list_
  page_table_.erase(page_id);

  page.ResetMemory();
  page.is_dirty_=false;
  page.page_id_=INVALID_PAGE_ID;
  page.pin_count_=0;


  DeallocatePage(page_id);
  return true;


}

auto BufferPoolManager::AllocatePage() -> page_id_t { return next_page_id_++; }

auto BufferPoolManager::FetchPageBasic(page_id_t page_id) -> BasicPageGuard { return {this, nullptr}; }

auto BufferPoolManager::FetchPageRead(page_id_t page_id) -> ReadPageGuard { return {this, nullptr}; }

auto BufferPoolManager::FetchPageWrite(page_id_t page_id) -> WritePageGuard { return {this, nullptr}; }

auto BufferPoolManager::NewPageGuarded(page_id_t *page_id) -> BasicPageGuard { return {this, nullptr}; }

}  // namespace bustub
