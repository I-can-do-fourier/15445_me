//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// b_plus_tree_sequential_scale_test.cpp
//
// Identification: test/storage/b_plus_tree_sequential_scale_test.cpp
//
// Copyright (c) 2023, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <random>

#include "buffer/buffer_pool_manager.h"
#include "gtest/gtest.h"
#include "storage/disk/disk_manager_memory.h"
#include "storage/index/b_plus_tree.h"
#include "test_util.h"  // NOLINT

namespace bustub {

using bustub::DiskManagerUnlimitedMemory;

/**
 * This test should be passing with your Checkpoint 1 submission.
 */
TEST(BPlusTreeTests, ScaleTest) {  // NOLINT
  // create KeyComparator and index schema
  auto key_schema = ParseCreateStatement("a bigint");
  GenericComparator<8> comparator(key_schema.get());

  auto disk_manager = std::make_unique<DiskManagerUnlimitedMemory>();
  auto *bpm = new BufferPoolManager(30, disk_manager.get());

  // create and fetch header_page
  page_id_t page_id;
  auto *header_page = bpm->NewPage(&page_id);
  (void)header_page;

  // create b+ tree
  BPlusTree<GenericKey<8>, RID, GenericComparator<8>> tree("foo_pk", page_id, bpm, comparator, 2, 3);
  GenericKey<8> index_key;
  RID rid;
  // create transaction
  auto *transaction = new Transaction(0);

  int64_t scale = 5000;
  std::vector<int64_t> keys;
  for (int64_t key = 1; key < scale; key++) {
    keys.push_back(key);
  }

  // randomized the insertion order
  auto rng = std::default_random_engine{};
  std::shuffle(keys.begin(), keys.end(), rng);
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
    ASSERT_EQ(rids.size(), 1);

    int64_t value = key & 0xFFFFFFFF;
    ASSERT_EQ(rids[0].GetSlotNum(), value);
  }

  bpm->UnpinPage(HEADER_PAGE_ID, true);
  delete transaction;
  delete bpm;
}

TEST(BPlusTreeTests, ScaleTest_My) {  // NOLINT
    // create KeyComparator and index schema
    auto key_schema = ParseCreateStatement("a bigint");
    GenericComparator<8> comparator(key_schema.get());

    auto disk_manager = std::make_unique<DiskManagerUnlimitedMemory>();
    auto *bpm = new BufferPoolManager(30, disk_manager.get());

    // create and fetch header_page
    page_id_t page_id;
    auto *header_page = bpm->NewPage(&page_id);
    (void)header_page;

    // create b+ tree
    //每个 internal page至少有两个entry
    BPlusTree<GenericKey<8>, RID, GenericComparator<8>> tree("foo_pk", page_id, bpm, comparator, 5, 4);
    GenericKey<8> index_key;
    RID rid;
    // create transaction
    auto *transaction = new Transaction(0);

    int64_t scale = 5000;
    std::vector<int64_t> keys;
    for (int64_t key = 1; key < scale; key++) {
        keys.push_back(key);
    }

    // randomized the insertion order
    auto rng = std::default_random_engine{};
    std::shuffle(keys.begin(), keys.end(), rng);

        GenericKey<8> init_key;
        init_key.SetFromInteger(4477);
        std::vector<RID> init_vec;
    for (auto key : keys) {
        init_vec.clear();

        int64_t value = key & 0xFFFFFFFF;
        rid.Set(static_cast<int32_t>(key >> 32), value);

        index_key.SetFromInteger(key);
        std::string g1;
        //if(tree.GetRootPageId()!=INVALID_PAGE_ID)g1=tree.DrawBPlusTree();

        tree.Insert(index_key, rid, transaction);
        //std::cout << tree.DrawBPlusTree() << "\n";
        //std::string g2=tree.DrawBPlusTree();
        tree.GetValue(init_key,&init_vec);
//        if(init_vec.size()==0){
//            std::cout << tree.DrawBPlusTree() << "\n";
//        };
//        if(key!=4477&&init_vec.size()==0){
//
//            std::cout << g1 << "\n\n\n";
//            std::cout << g2 << "\n\n\n";
//        }
    }
    std::vector<RID> rids;

    //std::cout << tree.DrawBPlusTree() << "\n";
    for (auto key : keys) {
        rids.clear();
        index_key.SetFromInteger(key);
        tree.GetValue(index_key, &rids);
        ASSERT_EQ(rids.size(), 1);

        int64_t value = key & 0xFFFFFFFF;
        ASSERT_EQ(rids[0].GetSlotNum(), value);
    }

    bpm->UnpinPage(HEADER_PAGE_ID, true);
    delete transaction;
    delete bpm;
}

TEST(BPlusTreeTests, ScaleTest_My2) {  // NOLINT
    // create KeyComparator and index schema
    auto key_schema = ParseCreateStatement("a bigint");
    GenericComparator<8> comparator(key_schema.get());

    auto disk_manager = std::make_unique<DiskManagerUnlimitedMemory>();
    auto *bpm = new BufferPoolManager(30, disk_manager.get());

    // create and fetch header_page
    page_id_t page_id;
    auto *header_page = bpm->NewPage(&page_id);
    (void)header_page;

    // create b+ tree
    //每个 internal page至少有两个entry
    BPlusTree<GenericKey<8>, RID, GenericComparator<8>> tree("foo_pk", page_id, bpm, comparator, 2, 3);
    GenericKey<8> index_key;
    RID rid;
    // create transaction
    auto *transaction = new Transaction(0);



    for (int64_t key=1;key<=5;key++) {

        int64_t value = key & 0xFFFFFFFF;
        rid.Set(static_cast<int32_t>(key >> 32), value);

        index_key.SetFromInteger(key);
        std::string g1;
     

        tree.Insert(index_key, rid, transaction);
        std::cout << tree.DrawBPlusTree() << "\n";
 
    }
    
    int64_t k=1;
    int64_t value = k & 0xFFFFFFFF;
    rid.Set(static_cast<int32_t>(k >> 32), value);
    index_key.SetFromInteger(k);
    auto res=tree.Insert(index_key, rid, transaction);

    std::cout<<res<<std::endl;
    bpm->UnpinPage(HEADER_PAGE_ID, true);
    delete transaction;
    delete bpm;
}

}  // namespace bustub
