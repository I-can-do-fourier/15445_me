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

namespace bustub {

LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : replacer_size_(num_frames), k_(k) {}

auto LRUKReplacer::Evict(frame_id_t *frame_id) -> bool { return false; }

void LRUKReplacer::RecordAccess(frame_id_t frame_id, [[maybe_unused]] AccessType access_type) {

 BUSTUB_ASSERT(frame_id>=0&&(uint32_t)frame_id<=replacer_size_,"invalid frame id");


 if(node_store_.find(frame_id)!=node_store_.end()){

   pq_.erase(frame_id);
   LRUKNode& node=node_store_[frame_id];
  //auto it=pq_.find(frame_id);
  node.GetHistory().push_back(current_timestamp_++);
  if(node.GetHistory().size()>9)node.GetHistory().pop_front();
  pq_.insert(frame_id);
 }
 else {

   LRUKNode node=LRUKNode{};

   node.GetK()=k_;
   node.GetFrameID()=frame_id;
   node.GetHistory().push_back(current_timestamp_++);


   node_store_[frame_id]=node;

   pq_.insert(frame_id);
 }

 //add fields


 //node_store_[frame_id]=node;

}

void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {

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
    curr_size_--;
  }

}

void LRUKReplacer::Remove(frame_id_t frame_id) {}

auto LRUKReplacer::Size() -> size_t { return 0; }

}  // namespace bustub
