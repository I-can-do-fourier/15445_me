//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.cpp
//
// Identification: src/buffer/lru_k_replacer.cpp
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_k_replacer.h"
#include "common/exception.h"

#include "buffer/log_helper.h"

namespace bustub {




LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : replacer_size_(num_frames), k_(k) {}

auto LRUKReplacer::Evict(frame_id_t *frame_id) -> bool {
  LOG("Evict",frame_id);
  if(curr_size_==0)return false;

  auto it=pq_.begin();
  frame_id_t eid=*it;

  *frame_id=eid;

  if(*frame_id<0)return false;//garbage


  pq_.erase(it);
  node_store_.erase(eid);
  curr_size_--;

  return true;
}

void LRUKReplacer::RecordAccess(frame_id_t frame_id, [[maybe_unused]] AccessType access_type) {
  LOG("RecordAccess",frame_id);
 BUSTUB_ASSERT(frame_id>=0&&(uint32_t)frame_id<=replacer_size_,"invalid frame id");


 if(node_store_.find(frame_id)!=node_store_.end()){



   LRUKNode& node=node_store_[frame_id];

   /**
    *
    * 潜在的bug。如果先改变history，之后再erase，然后立即insert。在erase过程中，是不是
    * 可能找不到这个element。
    */

   if(node.GetEvictable())pq_.erase(frame_id);
  //auto it=pq_.find(frame_id);
  node.GetHistory().push_back(current_timestamp_++);
  if(node.GetHistory().size()>k_)node.GetHistory().pop_front();

  if(node.GetEvictable())pq_.insert(frame_id);


 }
 else {

   LRUKNode node=LRUKNode{};

   node.GetK()=k_;
   node.GetFrameID()=frame_id;
   node.GetHistory().push_back(current_timestamp_++);


   node_store_[frame_id]=node;

   if(node.GetEvictable())pq_.insert(frame_id);
 }

 //add fields


 //node_store_[frame_id]=node;

}

void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {
  LOG("SetEvictable",frame_id,set_evictable);
  BUSTUB_ASSERT(frame_id>=0&&(uint32_t)frame_id<=replacer_size_,"invalid frame id");

  if(node_store_.find(frame_id)==node_store_.end())return;
  if(node_store_.at(frame_id).GetEvictable()==set_evictable)return;

  LRUKNode& node=node_store_[frame_id];
  node.GetEvictable()=set_evictable;

  if(set_evictable){

    pq_.insert(frame_id);
    curr_size_++;
  }else {

    pq_.erase(frame_id);
    //node_store_[frame_id].GetHistory().clear();//也就是说，frame被设置为non-evictable之后,所有history清空?
    curr_size_--;
  }

}

void LRUKReplacer::Remove(frame_id_t frame_id) {
  LOG("Remove",frame_id);
  BUSTUB_ASSERT(frame_id>=0&&(uint32_t)frame_id<=replacer_size_,"invalid frame id");

  if(node_store_.find(frame_id)==node_store_.end())return;

  LRUKNode& node=node_store_[frame_id];

  BUSTUB_ASSERT(node.GetEvictable(),"invalid frame id");

  node_store_.erase(frame_id);
  pq_.erase(frame_id);

  curr_size_--;
}

auto LRUKReplacer::Size() -> size_t {
  LOG("Size",-1);
  return curr_size_;

}



}  // namespace bustub
