//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// b_plus_tree_delete_test.cpp
//
// Identification: test/storage/b_plus_tree_delete_test.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <cstdio>

#include "buffer/buffer_pool_manager.h"
#include "gtest/gtest.h"
#include "storage/disk/disk_manager_memory.h"
#include "storage/index/b_plus_tree.h"
#include "test_util.h"  // NOLINT

#include <random>

namespace bustub {

using bustub::DiskManagerUnlimitedMemory;

TEST(BPlusTreeTests, DeleteTest1) {
  // create KeyComparator and index schema
  auto key_schema = ParseCreateStatement("a bigint");
  GenericComparator<8> comparator(key_schema.get());

  auto disk_manager = std::make_unique<DiskManagerUnlimitedMemory>();
  auto *bpm = new BufferPoolManager(50, disk_manager.get());
  // create and fetch header_page
  page_id_t page_id;
  auto header_page = bpm->NewPage(&page_id);
  // create b+ tree
  BPlusTree<GenericKey<8>, RID, GenericComparator<8>> tree("foo_pk", header_page->GetPageId(), bpm, comparator);
  GenericKey<8> index_key;
  RID rid;
  // create transaction
  auto *transaction = new Transaction(0);

  std::vector<int64_t> keys = {1, 2, 3, 4, 5};
  for (auto key : keys) {
    int64_t value = key & 0xFFFFFFFF;
    rid.Set(static_cast<int32_t>(key >> 32), value);
    index_key.SetFromInteger(key);
    tree.Insert(index_key, rid, transaction);
  }

  std::vector<RID> rids;
  for (auto key : keys) {
    rids.clear();
    index_key.SetFromInteger(key);
    tree.GetValue(index_key, &rids);
    EXPECT_EQ(rids.size(), 1);

    int64_t value = key & 0xFFFFFFFF;
    EXPECT_EQ(rids[0].GetSlotNum(), value);
  }

  std::vector<int64_t> remove_keys = {1, 5};
  for (auto key : remove_keys) {
    index_key.SetFromInteger(key);
    tree.Remove(index_key, transaction);
  }

  int64_t size = 0;
  bool is_present;

  for (auto key : keys) {
    rids.clear();
    index_key.SetFromInteger(key);
    is_present = tree.GetValue(index_key, &rids);

    if (!is_present) {
      EXPECT_NE(std::find(remove_keys.begin(), remove_keys.end(), key), remove_keys.end());
    } else {
      EXPECT_EQ(rids.size(), 1);
      EXPECT_EQ(rids[0].GetPageId(), 0);
      EXPECT_EQ(rids[0].GetSlotNum(), key);
      size = size + 1;
    }
  }

  EXPECT_EQ(size, 3);

  bpm->UnpinPage(HEADER_PAGE_ID, true);
  delete transaction;
  delete bpm;
}

TEST(BPlusTreeTests, DeleteTest2) {
  // create KeyComparator and index schema
  auto key_schema = ParseCreateStatement("a bigint");
  GenericComparator<8> comparator(key_schema.get());

  auto disk_manager = std::make_unique<DiskManagerUnlimitedMemory>();
  auto *bpm = new BufferPoolManager(50, disk_manager.get());
  // create and fetch header_page
  page_id_t page_id;
  auto header_page = bpm->NewPage(&page_id);
  // create b+ tree
  BPlusTree<GenericKey<8>, RID, GenericComparator<8>> tree("foo_pk", header_page->GetPageId(), bpm, comparator);
  GenericKey<8> index_key;
  RID rid;
  // create transaction
  auto *transaction = new Transaction(0);

  std::vector<int64_t> keys = {1, 2, 3, 4, 5};
  for (auto key : keys) {
    int64_t value = key & 0xFFFFFFFF;
    rid.Set(static_cast<int32_t>(key >> 32), value);
    index_key.SetFromInteger(key);
    tree.Insert(index_key, rid, transaction);
  }

  std::vector<RID> rids;
  for (auto key : keys) {
    rids.clear();
    index_key.SetFromInteger(key);
    tree.GetValue(index_key, &rids);
    EXPECT_EQ(rids.size(), 1);

    int64_t value = key & 0xFFFFFFFF;
    EXPECT_EQ(rids[0].GetSlotNum(), value);
  }

  std::vector<int64_t> remove_keys = {1, 5, 3, 4};
  for (auto key : remove_keys) {
    index_key.SetFromInteger(key);
    tree.Remove(index_key, transaction);
  }

  int64_t size = 0;
  bool is_present;

  for (auto key : keys) {
    rids.clear();
    index_key.SetFromInteger(key);
    is_present = tree.GetValue(index_key, &rids);

    if (!is_present) {
      EXPECT_NE(std::find(remove_keys.begin(), remove_keys.end(), key), remove_keys.end());
    } else {
      EXPECT_EQ(rids.size(), 1);
      EXPECT_EQ(rids[0].GetPageId(), 0);
      EXPECT_EQ(rids[0].GetSlotNum(), key);
      size = size + 1;
    }
  }

  EXPECT_EQ(size, 1);

  bpm->UnpinPage(HEADER_PAGE_ID, true);
  delete transaction;
  delete bpm;
}

/**
 *
 * 1.firstly, insert keys
 * 2.random insert/delete
 *
 * may repeat insert and delete void keys
 *
 *
 *
 */
TEST(BPlusTreeTests, DeleteTest2_me) {
  // create KeyComparator and index schema
  auto key_schema = ParseCreateStatement("a bigint");
  GenericComparator<8> comparator(key_schema.get());

  auto disk_manager = std::make_unique<DiskManagerUnlimitedMemory>();
  auto *bpm = new BufferPoolManager(50, disk_manager.get());
  // create and fetch header_page
  page_id_t page_id;
  auto header_page = bpm->NewPage(&page_id);
  // create b+ tree
  BPlusTree<GenericKey<8>, RID, GenericComparator<8>> tree("foo_pk", header_page->GetPageId(), bpm, comparator,20,16);
  GenericKey<8> index_key;
  RID rid;
  // create transaction
  auto *transaction = new Transaction(0);

  std::vector<int64_t> keys;

  std::unordered_set<int64_t> inserted;
  std::unordered_set<int64_t> outsides;

  std::vector<int> cmds_init;
  std::vector<int> cmds;


  for(int64_t i=0;i<5000;i++){

    keys.push_back(i);
    outsides.insert(i);
    if(std::rand()%7==1)keys.push_back(i);

    if(i<500)cmds.push_back(2);
    else if(i>=500&&i<1000)cmds.push_back(3);
    else if(i>=1000&&i<3000)cmds.push_back(1);
    else if(i>=3000)cmds.push_back(0);

  }

  for(int64_t i=0;i<500;i++){

    outsides.insert(5001+std::rand()%300);
    outsides.insert(-std::rand()%300);
  }


  std::cout<<inserted.max_size()<<std::endl;

  auto rng = std::default_random_engine{};

  std::vector<int64_t> t1{1,2,3,4,5,7,8};
  std::vector<int64_t> t2{1,2,3,4,5,7,8};

  std::shuffle(t1.begin(), t1.end(), rng);
  std::shuffle(t2.begin(), t2.end(), rng);

  std::shuffle(keys.begin(), keys.end(), rng);
  std::shuffle(cmds.begin(), cmds.end(), rng);



  for(int i=0;i<1000;i++) cmds.insert(cmds.begin(),1);


  auto it_keys=keys.begin();
  //auto it_cmds=cmds.begin();
  std::vector<RID> res;

  int count=0;
  for (auto cmd : cmds) {

    count++;
    int64_t key;
    if(cmd==1) {

      key=*it_keys;
      it_keys++;

      inserted.insert(key);
      outsides.erase(key);

      std::cout<<"to insert"<<" "<<key<<std::endl;
    }else if(cmd==0){





      auto it=inserted.begin();
      int index = std::rand() % inserted.size();
      //int index=0;
      std::advance(it,index);
      //std::cout<<*it<<std::endl;
      key=*it;
      //std::cout<<inserted.size()<<" "<<key<<std::endl;
      std::cout<<"to delete"<<" "<<key<<std::endl;
      inserted.erase(it);

      outsides.insert(key);
    }else if(cmd==2){

      int index = std::rand() % inserted.size();

      auto it_fake=inserted.begin();
      std::advance(it_fake,index);

      key=*it_fake;

      index_key.SetFromInteger(key);
      tree.GetValue(index_key, &res);

      std::cout<<"to insert repeat"<<" "<<key<<std::endl;

      if(key==858){

        tree.Draw(bpm, "../../my-tree.dot");
      }
      EXPECT_EQ(res.size(), 1);
      res.clear();

      inserted.insert(key);
      outsides.erase(key);
    }else{

      int index = std::rand() % outsides.size();

      auto it_fake=outsides.begin();
      std::advance(it_fake,index);

      key=*it_fake;

      index_key.SetFromInteger(key);
      tree.GetValue(index_key, &res);
      std::cout<<"to delete repeat"<<" "<<key<<std::endl;
      EXPECT_EQ(res.size(), 0);
      res.clear();

      inserted.erase(key);

    }

    int64_t value = key & 0xFFFFFFFF;

    rid.Set(static_cast<int32_t>(key >> 32), value);
    index_key.SetFromInteger(key);

    if(cmd==1||cmd==2){

      tree.Insert(index_key, rid, transaction);
      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 1);

      EXPECT_EQ(res[0].GetSlotNum(), value);
    }else{


      tree.GetValue(index_key, &res);
      if(cmd==0)EXPECT_EQ(res.size(), 1);
      else EXPECT_EQ(res.size(), 0);
      res.clear();


      tree.Remove(index_key, transaction);
      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 0);


    }

    res.clear();

  }

  for(auto key:inserted){

    index_key.SetFromInteger(key);
    int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 1);

    EXPECT_EQ(res[0].GetSlotNum(), value);

    res.clear();

  }

  for(auto key:keys){

      if(inserted.find(key)==inserted.end()){
        index_key.SetFromInteger(key);
        tree.GetValue(index_key, &res);
        EXPECT_EQ(res.size(), 0);

        res.clear();
      }
  }


  auto it_tree=tree.Begin();


  int64_t  pre=-1;
  int total=0;
  while(it_tree!=tree.End()){

      int64_t k=(*it_tree).first.ToString();

      std::cout<<"iterator:"<<k<<std::endl;

      auto t=inserted.find(k);

      int flag= (k-pre>0)? 1:0;
      EXPECT_EQ(flag, 1);
      ASSERT_NE(t, inserted.end());

      index_key.SetFromInteger(k);
      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 1);

      res.clear();

      total++;
      ++it_tree;
  }

  EXPECT_EQ(total,inserted.size());


  std::set<int64_t> order;

  for(int64_t t:inserted)order.insert(t);

  for(int i=0;i<20;i++){



    auto it=order.begin();
    int index = std::rand() % order.size();
    //int index=0;
    std::advance(it,index);
    //std::cout<<*it<<std::endl;
    int64_t key=*it;

    it=order.lower_bound(key);

    index_key.SetFromInteger(key);
    auto it_tree_mid=tree.Begin(index_key);

    while(it_tree_mid!=tree.End()){

      int64_t k=(*it_tree_mid).first.ToString();

      tree.GetValue((*it_tree_mid).first, &res);
      EXPECT_EQ(res.size(), 1);

      res.clear();

      int64_t kk=*it;
      EXPECT_EQ(k, kk);

      ++it_tree_mid;
      ++it;

    }

    EXPECT_EQ(it_tree_mid, tree.End());
    EXPECT_EQ(it, order.end());

  }

  std::vector<RID> rids;


  bpm->UnpinPage(HEADER_PAGE_ID, true);
  delete transaction;
  delete bpm;
}

/**
 *
 * 1.firstly, insert keys
 * 2.random insert/delete
 * 3. delete many keys
 * 4. insert keys
 *
 * may repeat insert and delete void keys
 *
 */

TEST(BPlusTreeTests, DeleteTest2_me2) {
  // create KeyComparator and index schema
  auto key_schema = ParseCreateStatement("a bigint");
  GenericComparator<8> comparator(key_schema.get());

  auto disk_manager = std::make_unique<DiskManagerUnlimitedMemory>();
  auto *bpm = new BufferPoolManager(50, disk_manager.get());
  // create and fetch header_page
  page_id_t page_id;
  auto header_page = bpm->NewPage(&page_id);
  // create b+ tree
  BPlusTree<GenericKey<8>, RID, GenericComparator<8>> tree("foo_pk", header_page->GetPageId(), bpm, comparator,93,42);
  GenericKey<8> index_key;
  RID rid;
  // create transaction
  auto *transaction = new Transaction(0);

  std::vector<int64_t> keys;

  std::unordered_set<int64_t> inserted;
  std::unordered_set<int64_t> outsides;

  std::vector<int> cmds_init;
  std::vector<int> cmds;


  for(int64_t i=0;i<5000;i++){

    keys.push_back(i);
    outsides.insert(i);
    if(std::rand()%7==1)keys.push_back(i);

    if(i<500)cmds.push_back(2);
    else if(i>=500&&i<1000)cmds.push_back(3);
    else if(i>=1000&&i<3000)cmds.push_back(1);
    else if(i>=3000)cmds.push_back(0);

  }

  for(int64_t i=0;i<500;i++){

    outsides.insert(5001+std::rand()%300);
    outsides.insert(-std::rand()%300);
  }


  std::cout<<inserted.max_size()<<std::endl;

  auto rng = std::default_random_engine{};

  std::vector<int64_t> t1{1,2,3,4,5,7,8};
  std::vector<int64_t> t2{1,2,3,4,5,7,8};

  std::shuffle(t1.begin(), t1.end(), rng);
  std::shuffle(t2.begin(), t2.end(), rng);

  std::shuffle(keys.begin(), keys.end(), rng);
  std::shuffle(cmds.begin(), cmds.end(), rng);



  for(int i=0;i<1000;i++) cmds.insert(cmds.begin(),1);


  auto it_keys=keys.begin();
  //auto it_cmds=cmds.begin();
  std::vector<RID> res;

  int count=0;
  for (auto cmd : cmds) {

    count++;
    int64_t key;
    if(cmd==1) {

      key=*it_keys;
      it_keys++;

      inserted.insert(key);
      outsides.erase(key);

      //std::cout<<"to insert"<<" "<<key<<std::endl;
    }else if(cmd==0){





      auto it=inserted.begin();
      int index = std::rand() % inserted.size();
      //int index=0;
      std::advance(it,index);
      //std::cout<<*it<<std::endl;
      key=*it;
      //std::cout<<inserted.size()<<" "<<key<<std::endl;
      //std::cout<<"to delete"<<" "<<key<<std::endl;
      inserted.erase(it);

      outsides.insert(key);
    }else if(cmd==2){

      int index = std::rand() % inserted.size();

      auto it_fake=inserted.begin();
      std::advance(it_fake,index);

      key=*it_fake;

      index_key.SetFromInteger(key);
      tree.GetValue(index_key, &res);

      //std::cout<<"to insert repeat"<<" "<<key<<std::endl;

      if(key==858){

        tree.Draw(bpm, "../../my-tree.dot");
      }
      EXPECT_EQ(res.size(), 1);
      res.clear();

      inserted.insert(key);
      outsides.erase(key);
    }else{

      int index = std::rand() % outsides.size();

      auto it_fake=outsides.begin();
      std::advance(it_fake,index);

      key=*it_fake;

      index_key.SetFromInteger(key);
      tree.GetValue(index_key, &res);
      //std::cout<<"to delete repeat"<<" "<<key<<std::endl;
      EXPECT_EQ(res.size(), 0);
      res.clear();

      inserted.erase(key);

    }

    int64_t value = key & 0xFFFFFFFF;

    rid.Set(static_cast<int32_t>(key >> 32), value);
    index_key.SetFromInteger(key);

    if(cmd==1||cmd==2){

      tree.Insert(index_key, rid, transaction);
      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 1);

      EXPECT_EQ(res[0].GetSlotNum(), value);
    }else{


      tree.GetValue(index_key, &res);
      if(cmd==0)EXPECT_EQ(res.size(), 1);
      else EXPECT_EQ(res.size(), 0);
      res.clear();


      tree.Remove(index_key, transaction);
      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 0);


    }

    res.clear();

  }

  for(auto key:inserted){

    index_key.SetFromInteger(key);
    int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 1);

    EXPECT_EQ(res[0].GetSlotNum(), value);

    res.clear();

  }

  for(auto key:outsides){

    index_key.SetFromInteger(key);
    //int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 0);

    //EXPECT_EQ(res[0].GetSlotNum(), value);

    //std::cout<<"inserted"<<" "<<key<<std::endl;
    res.clear();

  }

  for(int i=0;i<700;i++){

    int64_t key;

    auto it=inserted.begin();
    int index = std::rand() % inserted.size();
    //int index=0;
    std::advance(it,index);
    //std::cout<<*it<<std::endl;
    key=*it;
    index_key.SetFromInteger(key);
    //std::cout<<inserted.size()<<" "<<key<<std::endl;
    //std::cout<<"to delete"<<" "<<key<<std::endl;
    inserted.erase(it);

    outsides.insert(key);

    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 1);

    res.clear();

    tree.Remove(index_key, transaction);
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 0);


  }

  for(auto key:inserted){

    index_key.SetFromInteger(key);
    int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 1);

    EXPECT_EQ(res[0].GetSlotNum(), value);

    //std::cout<<"inserted"<<" "<<key<<std::endl;
    res.clear();

  }

  for(auto key:outsides){

    index_key.SetFromInteger(key);
    //int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 0);

    //EXPECT_EQ(res[0].GetSlotNum(), value);

    //std::cout<<"inserted"<<" "<<key<<std::endl;
    res.clear();

  }

  std::shuffle(keys.begin(), keys.end(), rng);
  it_keys=keys.begin();
  res.clear();
  for(int i=0;i<2000;i++){

    int64_t key=*it_keys;
    int64_t value = key & 0xFFFFFFFF;
    rid.Set(static_cast<int32_t>(key >> 32), value);

    index_key.SetFromInteger(key);
    //std::cout<<inserted.size()<<" "<<key<<std::endl;
    //std::cout<<"to insert"<<" "<<key<<std::endl;


    tree.GetValue(index_key, &res);
    if(res.size()==1){

      EXPECT_NE(inserted.find(key),inserted.end());
    }
    else {

      EXPECT_EQ(inserted.find(key),inserted.end());
    }

    res.clear();

    inserted.insert(key);
    outsides.erase(key);

    tree.Insert(index_key, rid,transaction);
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 1);
    res.clear();
    it_keys++;

  }

  for(auto key:inserted){

    index_key.SetFromInteger(key);
    int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 1);

    EXPECT_EQ(res[0].GetSlotNum(), value);

    //std::cout<<"inserted"<<" "<<key<<std::endl;
    res.clear();

  }

  for(auto key:outsides){

    index_key.SetFromInteger(key);
    //int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 0);

    //EXPECT_EQ(res[0].GetSlotNum(), value);

    //std::cout<<"inserted"<<" "<<key<<std::endl;
    res.clear();

  }



  for(auto key:keys){

    if(inserted.find(key)==inserted.end()){
      index_key.SetFromInteger(key);
      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 0);

      res.clear();
    }
  }


  auto it_tree=tree.Begin();


  int64_t  pre=-1;
  int total=0;
  while(it_tree!=tree.End()){

    int64_t k=(*it_tree).first.ToString();

    //std::cout<<"iterator:"<<k<<std::endl;

    auto t=inserted.find(k);

    int flag= (k-pre>0)? 1:0;
    EXPECT_EQ(flag, 1);
    EXPECT_NE(t, inserted.end());

    index_key.SetFromInteger(k);
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 1);

    res.clear();

    total++;
    ++it_tree;
  }

  EXPECT_EQ(total,inserted.size());


  std::set<int64_t> order;

  for(int64_t t:inserted)order.insert(t);

  for(int i=0;i<20;i++){



    auto it=order.begin();
    int index = std::rand() % order.size();
    //int index=0;
    std::advance(it,index);
    //std::cout<<*it<<std::endl;
    int64_t key=*it;

    it=order.lower_bound(key);

    index_key.SetFromInteger(key);
    auto it_tree_mid=tree.Begin(index_key);

    while(it_tree_mid!=tree.End()){

      int64_t k=(*it_tree_mid).first.ToString();

      tree.GetValue((*it_tree_mid).first, &res);
      EXPECT_EQ(res.size(), 1);

      res.clear();

      int64_t kk=*it;
      EXPECT_EQ(k, kk);

      ++it_tree_mid;
      ++it;

    }

    EXPECT_EQ(it_tree_mid, tree.End());
    EXPECT_EQ(it, order.end());

  }

  std::vector<RID> rids;


  bpm->UnpinPage(HEADER_PAGE_ID, true);
  delete transaction;
  delete bpm;
}


/**
 *
 * 1.firstly, insert keys
 * 2.random insert/delete
 * 3. delete many keys
 * 4. repeat step 2
 *
 * may repeat insert and delete void keys
 *
 */

TEST(BPlusTreeTests, DeleteTest2_me3) {
  // create KeyComparator and index schema
  auto key_schema = ParseCreateStatement("a bigint");
  GenericComparator<8> comparator(key_schema.get());

  auto disk_manager = std::make_unique<DiskManagerUnlimitedMemory>();
  auto *bpm = new BufferPoolManager(50, disk_manager.get());
  // create and fetch header_page
  page_id_t page_id;
  auto header_page = bpm->NewPage(&page_id);
  // create b+ tree
  BPlusTree<GenericKey<8>, RID, GenericComparator<8>> tree("foo_pk", header_page->GetPageId(), bpm, comparator,3,3);
  GenericKey<8> index_key;
  RID rid;
  // create transaction
  auto *transaction = new Transaction(0);

  std::vector<int64_t> keys;

  std::unordered_set<int64_t> inserted;
  std::unordered_set<int64_t> outsides;

  std::vector<int> cmds_init;
  std::vector<int> cmds;


  for(int64_t i=0;i<5000;i++){

    keys.push_back(i);
    outsides.insert(i);
    if(std::rand()%7==1)keys.push_back(i);

    if(i<500)cmds.push_back(2);
    else if(i>=500&&i<1000)cmds.push_back(3);
    else if(i>=1000&&i<3000)cmds.push_back(1);
    else if(i>=3000)cmds.push_back(0);

  }

  for(int64_t i=0;i<500;i++){

    outsides.insert(5001+std::rand()%300);
    outsides.insert(-std::rand()%300);
  }


  std::cout<<inserted.max_size()<<std::endl;

  auto rng = std::default_random_engine{};

  std::vector<int64_t> t1{1,2,3,4,5,7,8};
  std::vector<int64_t> t2{1,2,3,4,5,7,8};

  std::shuffle(t1.begin(), t1.end(), rng);
  std::shuffle(t2.begin(), t2.end(), rng);

  std::shuffle(keys.begin(), keys.end(), rng);
  std::shuffle(cmds.begin(), cmds.end(), rng);



  for(int i=0;i<1000;i++) cmds.insert(cmds.begin(),1);


  auto it_keys=keys.begin();
  //auto it_cmds=cmds.begin();
  std::vector<RID> res;

  int count=0;
  for (auto cmd : cmds) {

    count++;
    int64_t key;
    if(cmd==1) {

      key=*it_keys;
      it_keys++;

      inserted.insert(key);
      outsides.erase(key);

      //std::cout<<"to insert"<<" "<<key<<std::endl;
    }else if(cmd==0){





      auto it=inserted.begin();
      int index = std::rand() % inserted.size();
      //int index=0;
      std::advance(it,index);
      //std::cout<<*it<<std::endl;
      key=*it;
      //std::cout<<inserted.size()<<" "<<key<<std::endl;
      //std::cout<<"to delete"<<" "<<key<<std::endl;
      inserted.erase(it);

      outsides.insert(key);
    }else if(cmd==2){

      int index = std::rand() % inserted.size();

      auto it_fake=inserted.begin();
      std::advance(it_fake,index);

      key=*it_fake;

      index_key.SetFromInteger(key);
      tree.GetValue(index_key, &res);

      //std::cout<<"to insert repeat"<<" "<<key<<std::endl;

      if(key==858){

        tree.Draw(bpm, "../../my-tree.dot");
      }
      EXPECT_EQ(res.size(), 1);
      res.clear();

      inserted.insert(key);
      outsides.erase(key);
    }else{

      int index = std::rand() % outsides.size();

      auto it_fake=outsides.begin();
      std::advance(it_fake,index);

      key=*it_fake;

      index_key.SetFromInteger(key);
      tree.GetValue(index_key, &res);
      //std::cout<<"to delete repeat"<<" "<<key<<std::endl;
      EXPECT_EQ(res.size(), 0);
      res.clear();

      inserted.erase(key);

    }

    int64_t value = key & 0xFFFFFFFF;

    rid.Set(static_cast<int32_t>(key >> 32), value);
    index_key.SetFromInteger(key);

    if(cmd==1||cmd==2){

      tree.Insert(index_key, rid, transaction);
      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 1);

      EXPECT_EQ(res[0].GetSlotNum(), value);
    }else{


      tree.GetValue(index_key, &res);
      if(cmd==0)EXPECT_EQ(res.size(), 1);
      else EXPECT_EQ(res.size(), 0);
      res.clear();


      tree.Remove(index_key, transaction);
      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 0);


    }

    res.clear();

  }

  for(auto key:inserted){

    index_key.SetFromInteger(key);
    int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 1);

    EXPECT_EQ(res[0].GetSlotNum(), value);

    res.clear();

  }

  for(auto key:outsides){

    index_key.SetFromInteger(key);
    //int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 0);

    //EXPECT_EQ(res[0].GetSlotNum(), value);

    //std::cout<<"inserted"<<" "<<key<<std::endl;
    res.clear();

  }

  for(int i=0;i<700;i++){

    int64_t key;

    auto it=inserted.begin();
    int index = std::rand() % inserted.size();
    //int index=0;
    std::advance(it,index);
    //std::cout<<*it<<std::endl;
    key=*it;
    index_key.SetFromInteger(key);
    //std::cout<<inserted.size()<<" "<<key<<std::endl;
    //std::cout<<"to delete"<<" "<<key<<std::endl;
    inserted.erase(it);

    outsides.insert(key);

    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 1);

    res.clear();

    tree.Remove(index_key, transaction);
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 0);


  }

  for(auto key:inserted){

    index_key.SetFromInteger(key);
    int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 1);

    EXPECT_EQ(res[0].GetSlotNum(), value);

    //std::cout<<"inserted"<<" "<<key<<std::endl;
    res.clear();

  }

  for(auto key:outsides){

    index_key.SetFromInteger(key);
    //int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 0);

    //EXPECT_EQ(res[0].GetSlotNum(), value);

    //std::cout<<"inserted"<<" "<<key<<std::endl;
    res.clear();

  }

  std::shuffle(keys.begin(), keys.end(), rng);
  std::shuffle(cmds.begin(), cmds.end(), rng);
  it_keys=keys.begin();
  res.clear();

  for (auto cmd : cmds) {

    count++;
    int64_t key;
    if(cmd==1) {

      key=*it_keys;
      it_keys++;

      inserted.insert(key);
      outsides.erase(key);

      //std::cout<<"to insert"<<" "<<key<<std::endl;
    }else if(cmd==0){





      auto it=inserted.begin();
      int index = std::rand() % inserted.size();
      //int index=0;
      std::advance(it,index);
      //std::cout<<*it<<std::endl;
      key=*it;
      //std::cout<<inserted.size()<<" "<<key<<std::endl;
      //std::cout<<"to delete"<<" "<<key<<std::endl;
      inserted.erase(it);

      outsides.insert(key);
    }else if(cmd==2){

      int index = std::rand() % inserted.size();

      auto it_fake=inserted.begin();
      std::advance(it_fake,index);

      key=*it_fake;

      index_key.SetFromInteger(key);
      tree.GetValue(index_key, &res);

      //std::cout<<"to insert repeat"<<" "<<key<<std::endl;

      if(key==858){

        tree.Draw(bpm, "../../my-tree.dot");
      }
      EXPECT_EQ(res.size(), 1);
      res.clear();

      inserted.insert(key);
      outsides.erase(key);
    }else{

      int index = std::rand() % outsides.size();

      auto it_fake=outsides.begin();
      std::advance(it_fake,index);

      key=*it_fake;

      index_key.SetFromInteger(key);
      tree.GetValue(index_key, &res);
      //std::cout<<"to delete repeat"<<" "<<key<<std::endl;
      EXPECT_EQ(res.size(), 0);
      res.clear();

      inserted.erase(key);

    }

    int64_t value = key & 0xFFFFFFFF;

    rid.Set(static_cast<int32_t>(key >> 32), value);
    index_key.SetFromInteger(key);

    if(cmd==1||cmd==2){

      tree.Insert(index_key, rid, transaction);
      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 1);

      EXPECT_EQ(res[0].GetSlotNum(), value);
    }else{


      tree.GetValue(index_key, &res);
      if(cmd==0)EXPECT_EQ(res.size(), 1);
      else EXPECT_EQ(res.size(), 0);
      res.clear();


      tree.Remove(index_key, transaction);
      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 0);


    }

    res.clear();

  }

  for(auto key:inserted){

    index_key.SetFromInteger(key);
    int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 1);

    EXPECT_EQ(res[0].GetSlotNum(), value);

    //std::cout<<"inserted"<<" "<<key<<std::endl;
    res.clear();

  }

  for(auto key:outsides){

    index_key.SetFromInteger(key);
    //int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 0);

    //EXPECT_EQ(res[0].GetSlotNum(), value);

    //std::cout<<"inserted"<<" "<<key<<std::endl;
    res.clear();

  }



  for(auto key:keys){

    if(inserted.find(key)==inserted.end()){
      index_key.SetFromInteger(key);
      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 0);

      res.clear();
    }
  }


  auto it_tree=tree.Begin();


  int64_t  pre=-1;
  int total=0;
  while(it_tree!=tree.End()){

    int64_t k=(*it_tree).first.ToString();

    //std::cout<<"iterator:"<<k<<std::endl;

    auto t=inserted.find(k);

    int flag= (k-pre>0)? 1:0;
    EXPECT_EQ(flag, 1);
    EXPECT_NE(t, inserted.end());

    index_key.SetFromInteger(k);
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 1);

    res.clear();

    total++;
    ++it_tree;
  }

  EXPECT_EQ(total,inserted.size());


  std::set<int64_t> order;

  for(int64_t t:inserted)order.insert(t);

  int64_t min_number=*(order.begin())-100;
  int64_t max_number=6500;
  for(int64_t i=min_number;i<=max_number;i++){



//    auto it=order.begin();
//    int index = std::rand() % order.size();
//    //int index=0;
//    std::advance(it,index);
    //std::cout<<*it<<std::endl;
    int64_t key=i;

    auto it=order.lower_bound(key);

    index_key.SetFromInteger(key);
    auto it_tree_mid=tree.Begin(index_key);

    while(it_tree_mid!=tree.End()){

      int64_t k=(*it_tree_mid).first.ToString();

      tree.GetValue((*it_tree_mid).first, &res);
      EXPECT_EQ(res.size(), 1);

      res.clear();

      int64_t kk=*it;
      EXPECT_EQ(k, kk);

      ++it_tree_mid;
      ++it;

    }

    EXPECT_EQ(it_tree_mid, tree.End());
    EXPECT_EQ(it, order.end());

  }

  std::vector<RID> rids;


  bpm->UnpinPage(HEADER_PAGE_ID, true);
  delete transaction;
  delete bpm;
}

/**
 *
 * 1.firstly, insert keys
 * 2.random insert/delete
 * 3. delete all keys,then continue deletion
 * 4. insert some,then repeat step3
 *
 * may repeat insert and delete void keys
 *
 */

TEST(BPlusTreeTests, DeleteTest2_me4) {
  // create KeyComparator and index schema
  auto key_schema = ParseCreateStatement("a bigint");
  GenericComparator<8> comparator(key_schema.get());

  auto disk_manager = std::make_unique<DiskManagerUnlimitedMemory>();
  auto *bpm = new BufferPoolManager(50, disk_manager.get());
  // create and fetch header_page
  page_id_t page_id;
  auto header_page = bpm->NewPage(&page_id);
  // create b+ tree
  BPlusTree<GenericKey<8>, RID, GenericComparator<8>> tree("foo_pk", header_page->GetPageId(), bpm, comparator,102,93);
  GenericKey<8> index_key;
  RID rid;
  // create transaction
  auto *transaction = new Transaction(0);

  std::vector<int64_t> keys;

  std::unordered_set<int64_t> inserted;
  std::unordered_set<int64_t> outsides;

  std::vector<int> cmds_init;
  std::vector<int> cmds;


  for(int64_t i=0;i<5000;i++){

    keys.push_back(i);
    outsides.insert(i);
    if(std::rand()%7==1)keys.push_back(i);

    if(i<500)cmds.push_back(2);
    else if(i>=500&&i<1000)cmds.push_back(3);
    else if(i>=1000&&i<3000)cmds.push_back(1);
    else if(i>=3000)cmds.push_back(0);

  }

  for(int64_t i=0;i<500;i++){

    outsides.insert(5001+std::rand()%300);
    outsides.insert(-std::rand()%300);
  }


  std::cout<<inserted.max_size()<<std::endl;

  auto rng = std::default_random_engine{};

  std::vector<int64_t> t1{1,2,3,4,5,7,8};
  std::vector<int64_t> t2{1,2,3,4,5,7,8};

  std::shuffle(t1.begin(), t1.end(), rng);
  std::shuffle(t2.begin(), t2.end(), rng);

  std::shuffle(keys.begin(), keys.end(), rng);
  std::shuffle(cmds.begin(), cmds.end(), rng);



  for(int i=0;i<1000;i++) cmds.insert(cmds.begin(),1);


  auto it_keys=keys.begin();
  //auto it_cmds=cmds.begin();
  std::vector<RID> res;

  int count=0;
  for (auto cmd : cmds) {

    count++;
    int64_t key;
    if(cmd==1) {

      key=*it_keys;
      it_keys++;

      inserted.insert(key);
      outsides.erase(key);

      //std::cout<<"to insert"<<" "<<key<<std::endl;
    }else if(cmd==0){





      auto it=inserted.begin();
      int index = std::rand() % inserted.size();
      //int index=0;
      std::advance(it,index);
      //std::cout<<*it<<std::endl;
      key=*it;
      //std::cout<<inserted.size()<<" "<<key<<std::endl;
      //std::cout<<"to delete"<<" "<<key<<std::endl;
      inserted.erase(it);

      outsides.insert(key);
    }else if(cmd==2){

      int index = std::rand() % inserted.size();

      auto it_fake=inserted.begin();
      std::advance(it_fake,index);

      key=*it_fake;

      index_key.SetFromInteger(key);
      tree.GetValue(index_key, &res);

      //std::cout<<"to insert repeat"<<" "<<key<<std::endl;

      if(key==858){

        tree.Draw(bpm, "../../my-tree.dot");
      }
      EXPECT_EQ(res.size(), 1);
      res.clear();

      inserted.insert(key);
      outsides.erase(key);
    }else{

      int index = std::rand() % outsides.size();

      auto it_fake=outsides.begin();
      std::advance(it_fake,index);

      key=*it_fake;

      index_key.SetFromInteger(key);
      tree.GetValue(index_key, &res);
      //std::cout<<"to delete repeat"<<" "<<key<<std::endl;
      EXPECT_EQ(res.size(), 0);
      res.clear();

      inserted.erase(key);

    }

    int64_t value = key & 0xFFFFFFFF;

    rid.Set(static_cast<int32_t>(key >> 32), value);
    index_key.SetFromInteger(key);

    if(cmd==1||cmd==2){

      tree.Insert(index_key, rid, transaction);
      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 1);

      EXPECT_EQ(res[0].GetSlotNum(), value);
    }else{


      tree.GetValue(index_key, &res);
      if(cmd==0)EXPECT_EQ(res.size(), 1);
      else EXPECT_EQ(res.size(), 0);
      res.clear();


      tree.Remove(index_key, transaction);
      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 0);


    }

    res.clear();

  }

  for(auto key:inserted){

    index_key.SetFromInteger(key);
    int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 1);

    EXPECT_EQ(res[0].GetSlotNum(), value);

    res.clear();

  }

  for(auto key:outsides){

    index_key.SetFromInteger(key);
    //int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 0);

    //EXPECT_EQ(res[0].GetSlotNum(), value);

    //std::cout<<"inserted"<<" "<<key<<std::endl;
    res.clear();

  }

  //delete all
  for(int i=0;i<3000;i++){

    if(inserted.size()==0)break;
    int64_t key;

    auto it=inserted.begin();
    int index = std::rand() % inserted.size();
    //int index=0;
    std::advance(it,index);
    //std::cout<<*it<<std::endl;
    key=*it;
    index_key.SetFromInteger(key);
    //std::cout<<inserted.size()<<" "<<key<<std::endl;
    //std::cout<<"to delete"<<" "<<key<<std::endl;
    inserted.erase(it);

    outsides.insert(key);

    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 1);

    res.clear();

    tree.Remove(index_key, transaction);
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 0);


  }

  //delete extra


  for(int tt=0;tt<10;tt++){

    for(int i=0;i<100;i++){

      int64_t key;

      auto it=outsides.begin();
      int index = std::rand() % outsides.size();
      //int index=0;
      std::advance(it,index);
      //std::cout<<*it<<std::endl;
      key=*it;
      index_key.SetFromInteger(key);
      //std::cout<<inserted.size()<<" "<<key<<std::endl;
      //std::cout<<"to delete"<<" "<<key<<std::endl;
      inserted.erase(key);

      outsides.insert(key);

      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 0);

      res.clear();

      tree.Remove(index_key, transaction);
      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 0);


    }

    for(int i=0;i<200;i++){


      int64_t key;

      auto it=outsides.begin();
      int index = std::rand() % outsides.size();
      //int index=0;
      std::advance(it,index);
      //std::cout<<*it<<std::endl;
      key=*it;

      //if(key<0)continue;

      inserted.insert(key);
      outsides.erase(key);

      index_key.SetFromInteger(key);


      int64_t value = key & 0xFFFFFFFF;
      rid.Set(static_cast<int32_t>(key >> 32), value);

      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 0);
      res.clear();

      tree.Insert(index_key, rid, transaction);
      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 1);

      EXPECT_EQ(res[0].GetSlotNum(), value);
      res.clear();

    }

    for(int i=0;i<300;i++){

      if(inserted.size()==0)break;
      int64_t key;

      auto it=inserted.begin();
      int index = std::rand() % inserted.size();
      //int index=0;
      std::advance(it,index);
      //std::cout<<*it<<std::endl;
      key=*it;
      index_key.SetFromInteger(key);
      //std::cout<<inserted.size()<<" "<<key<<std::endl;
      //std::cout<<"to delete"<<" "<<key<<std::endl;
      inserted.erase(it);

      outsides.insert(key);

      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 1);

      res.clear();

      tree.Remove(index_key, transaction);
      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 0);

      res.clear();
    }

  }


  for(auto key:inserted){

    index_key.SetFromInteger(key);
    int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 1);

    EXPECT_EQ(res[0].GetSlotNum(), value);

    //std::cout<<"inserted"<<" "<<key<<std::endl;
    res.clear();

  }

  for(auto key:outsides){

    index_key.SetFromInteger(key);
    //int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 0);

    //EXPECT_EQ(res[0].GetSlotNum(), value);

    //std::cout<<"inserted"<<" "<<key<<std::endl;
    res.clear();

  }

  std::shuffle(keys.begin(), keys.end(), rng);
  std::shuffle(cmds.begin(), cmds.end(), rng);
  it_keys=keys.begin();
  res.clear();




  for(auto key:inserted){

    index_key.SetFromInteger(key);
    int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 1);

    EXPECT_EQ(res[0].GetSlotNum(), value);

    //std::cout<<"inserted"<<" "<<key<<std::endl;
    res.clear();

  }

  for(auto key:outsides){

    index_key.SetFromInteger(key);
    //int64_t value = key & 0xFFFFFFFF;
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 0);

    //EXPECT_EQ(res[0].GetSlotNum(), value);

    //std::cout<<"inserted"<<" "<<key<<std::endl;
    res.clear();

  }



  for(auto key:keys){

    if(inserted.find(key)==inserted.end()){
      index_key.SetFromInteger(key);
      tree.GetValue(index_key, &res);
      EXPECT_EQ(res.size(), 0);

      res.clear();
    }
  }


  auto it_tree=tree.Begin();


  int64_t  pre=-1;
  int total=0;
  while(it_tree!=tree.End()){

    int64_t k=(*it_tree).first.ToString();

    //std::cout<<"iterator:"<<k<<std::endl;

    auto t=inserted.find(k);

    int flag= (k-pre>0)? 1:0;
    EXPECT_EQ(flag, 1);
    EXPECT_NE(t, inserted.end());

    index_key.SetFromInteger(k);
    tree.GetValue(index_key, &res);
    EXPECT_EQ(res.size(), 1);

    res.clear();

    total++;
    ++it_tree;
  }

  EXPECT_EQ(total,inserted.size());

  std::set<int64_t> order;

  for(int64_t t:inserted)order.insert(t);

  int64_t min_number=-200;
  int64_t max_number=6500;
  for(int64_t i=min_number;i<=max_number;i++){



    //    auto it=order.begin();
    //    int index = std::rand() % order.size();
    //    //int index=0;
    //    std::advance(it,index);
    //std::cout<<*it<<std::endl;
    int64_t key=i;

    auto it=order.lower_bound(key);

    index_key.SetFromInteger(key);
    auto it_tree_mid=tree.Begin(index_key);

    while(it_tree_mid!=tree.End()){

      int64_t k=(*it_tree_mid).first.ToString();

      tree.GetValue((*it_tree_mid).first, &res);
      EXPECT_EQ(res.size(), 1);

      res.clear();

      int64_t kk=*it;
      EXPECT_EQ(k, kk);

      ++it_tree_mid;
      ++it;

    }

    EXPECT_EQ(it_tree_mid, tree.End());
    EXPECT_EQ(it, order.end());

  }

  std::vector<RID> rids;


  bpm->UnpinPage(HEADER_PAGE_ID, true);
  delete transaction;
  delete bpm;
}

}  // namespace bustub
