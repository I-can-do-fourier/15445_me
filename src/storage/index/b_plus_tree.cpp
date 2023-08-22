#include <sstream>
#include <string>

#include "common/config.h"
#include "common/exception.h"
#include "common/logger.h"
#include "common/rid.h"
#include "storage/index/b_plus_tree.h"
#include "storage/page/b_plus_tree_internal_page.h"
#include "storage/page/page_guard.h"

bool g_enable_logging_tree = true;

namespace bustub {

INDEX_TEMPLATE_ARGUMENTS
BPLUSTREE_TYPE::BPlusTree(std::string name, page_id_t header_page_id, BufferPoolManager *buffer_pool_manager,
                          const KeyComparator &comparator, int leaf_max_size, int internal_max_size)
    : index_name_(std::move(name)),
      bpm_(buffer_pool_manager),
      comparator_(std::move(comparator)),
      leaf_max_size_(leaf_max_size),
      internal_max_size_(internal_max_size),
      header_page_id_(header_page_id) {

    LogTree("BPlusTree","leaf_max_size_:",leaf_max_size,"internal_max_size_:",internal_max_size);

  WritePageGuard guard = bpm_->FetchPageWrite(header_page_id_);
  auto root_page = guard.AsMut<BPlusTreeHeaderPage>();
  root_page->root_page_id_ = INVALID_PAGE_ID;
}

/*
 * Helper function to decide whether current b+tree is empty
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::IsEmpty() const -> bool { return false; }
/*****************************************************************************
 * SEARCH
 *****************************************************************************/
/*
 * Return the only value that associated with input key
 * This method is used for point query
 * @return : true means key exists
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::GetValue(const KeyType &key, std::vector<ValueType> *result, Transaction *txn) -> bool {
  // Declaration of context instance.
  Context ctx;
  (void)ctx;


  BasicPageGuard hd_guard = bpm_->FetchPageBasic(header_page_id_);
  auto header_page = hd_guard.AsMut<BPlusTreeHeaderPage>();

  page_id_t pid=header_page->root_page_id_;

  if(pid==INVALID_PAGE_ID) return false;

  
  auto res=GetValueHp(key,result,txn,pid,ctx);
  
  return res;
}

//self-defined

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::GetValueHp(const KeyType &key, std::vector<ValueType> *result,Transaction *txn,page_id_t &page_id,Context &ctx) -> bool {

  auto guard=bpm_->FetchPageBasic(page_id);

  auto p = guard.AsMut<BPlusTreePage>();

  if(p->IsLeafPage()){

    auto page=reinterpret_cast<BPlusTreeLeafPage<KeyType,ValueType,KeyComparator> *>(guard.GetDataMut());

    auto index=page->Search(key, comparator_);

    if(index<0)return false;

    result->push_back(page->Get(index)) ;

    return true;
  }else{

   
    auto page=reinterpret_cast<BPlusTreeInternalPage<KeyType,page_id_t,KeyComparator> *>(guard.GetDataMut());

    auto index= page->Search(key,comparator_);

    
    if(index<0)return false;
    //page->Temp();
    //page->ToString();

    auto pid=page->GetPointer(index);
    if(pid==INVALID_PAGE_ID)return false;

    return GetValueHp(key, result, txn, pid, ctx);

    


  }

  
  return false;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Insert constant key & value pair into b+ tree
 * if current tree is empty, start new tree, update root page id and insert
 * entry, otherwise insert into leaf page.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Insert(const KeyType &key, const ValueType &value, Transaction *txn) -> bool {
  // Declaration of context instance.
  Context ctx;
  (void)ctx;

  LogTree("Insert","key:",key.ToString(),"value:",value.ToString());
//  auto hd_guard=bpm_->FetchPageBasic(header_page_id_);
//  auto it=reinterpret_cast<BPlusTreeHeaderPage *>(hd_guard.GetDataMut());

  BasicPageGuard hd_guard = bpm_->FetchPageBasic(header_page_id_);
  auto header_page = hd_guard.AsMut<BPlusTreeHeaderPage>();

  page_id_t pid=header_page->root_page_id_;

  BasicPageGuard guard;
  if(pid==INVALID_PAGE_ID){



    guard=bpm_->NewPageGuarded(&pid);
    header_page->root_page_id_=pid;
    auto page=reinterpret_cast<BPlusTreeLeafPage<KeyType,ValueType,KeyComparator> *>(guard.GetDataMut());
    page->Init(leaf_max_size_);
    guard.Drop();

  }

  ctx.root_page_id_=pid;
  //ctx.header_page_=std::move(hd_guard);
  auto pair=InsertHp(key,value,txn,pid,ctx);//这个地方要用reference

  if(pair.second==INVALID_PAGE_ID)return false;

  if(pair.second!=pid){

    page_id_t new_pid;
    guard=bpm_->NewPageGuarded(&new_pid);
    //header_page->root_page_id_=pid;
    auto page=reinterpret_cast<BPlusTreeInternalPage<KeyType,page_id_t,KeyComparator> *>(guard.GetDataMut());
    page->Init(internal_max_size_);
    page->Insert(0,pair.first, pair.second,comparator_);
    page->Insert(0,KeyType{1},pid,comparator_);

    guard.Drop();

    header_page->root_page_id_=new_pid;
  }
  return true;
}

//SELF-DEFINED
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::InsertHp(const KeyType &key, const ValueType &value, Transaction *txn,page_id_t &page_id,Context &ctx) -> std::pair<KeyType,page_id_t>{
  // Declaration of context instance.


  auto guard=bpm_->FetchPageBasic(page_id);

  auto p = guard.AsMut<BPlusTreePage>();

  if(p->IsLeafPage()){

    auto page=reinterpret_cast<BPlusTreeLeafPage<KeyType,ValueType,KeyComparator> *>(guard.GetDataMut());

    auto res=page->Insert(key, value,comparator_);

    if(!res)return std::pair<KeyType,page_id_t>(key,INVALID_PAGE_ID);

    if(page->GetSize()==page->GetMaxSize()){

        auto new_p=page->Split(bpm_);

        return new_p;
    }


    return std::pair<KeyType,page_id_t>(key,page_id);

  }else{

    /**

      因为BPLUSTREE_TYPE和BPlusTreeInternalPage的explicit template instantiation不同，
      这里的ValueType要明确成page_id_t
    */
    auto page=reinterpret_cast<BPlusTreeInternalPage<KeyType,page_id_t,KeyComparator> *>(guard.GetDataMut());

    auto index= page->Search(key,comparator_);

    if(index<0)return std::pair<KeyType,page_id_t>(key,INVALID_PAGE_ID);
    //page->Temp();
    //page->ToString();

    auto pid=page->GetPointer(index);
    if(pid==INVALID_PAGE_ID)return std::pair<KeyType,page_id_t>(key,pid);

    auto pair=InsertHp(key,value,txn,pid,ctx);

    if(pair.second==INVALID_PAGE_ID)return std::pair<KeyType,page_id_t>(key,pair.second); 
    
    if(pair.second!=pid){
      // page_id_t* new_pid;
      // BasicPageGuard new_guard=bpm_->NewPageGuarded(new_pid);
      // auto new_page=reinterpret_cast<BPlusTreeInternalPage<KeyType,ValueType,KeyComparator> *>(new_guard.GetDataMut());


        if(page->GetSize()==page->GetMaxSize()){


            auto new_p=page->SplitAndInsert(bpm_,index,pair.first,pair.second,comparator_);



            return new_p;
        }else page->Insert(index+1,pair.first,pair.second,comparator_);





      return std::pair<KeyType,page_id_t>(key,page_id);
      
    }else{


      return std::pair<KeyType,page_id_t>(key,page_id);
    }





  }

  return std::pair<KeyType,page_id_t>(key,page_id);

}



INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Search(const KeyType &key, MappingType* array, Transaction *txn,page_id_t &page_id,Context &ctx) -> int{
  // Declaration of context instance.
 

 return 0;

}

//END-DEFINED

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Delete key & value pair associated with input key
 * If current tree is empty, return immediately.
 * If not, User needs to first find the right leaf page as deletion target, then
 * delete entry from leaf page. Remember to deal with redistribute or merge if
 * necessary.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Remove(const KeyType &key, Transaction *txn) {
  // Declaration of context instance.
  Context ctx;
  (void)ctx;

  BasicPageGuard hd_guard = bpm_->FetchPageBasic(header_page_id_);
  auto header_page = hd_guard.AsMut<BPlusTreeHeaderPage>();

  page_id_t pid=header_page->root_page_id_;

  BasicPageGuard guard;
  if(pid==INVALID_PAGE_ID)return;

  ctx.root_page_id_=pid;
  //ctx.header_page_=std::move(hd_guard);

    
   auto gd=bpm_->FetchPageBasic(pid);

   auto p = guard.AsMut<BPlusTreePage>();

  RemoveHp(key,txn,p,ctx);//这个地方要用reference


}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::RemoveHp(const KeyType &key, Transaction *txn,BPlusTreePage* p,Context &ctx){

  

  //auto guard=bpm_->FetchPageBasic(page_id);

 // auto p = guard.AsMut<BPlusTreePage>();

  if(p->IsLeafPage()){

    auto page=reinterpret_cast<BPlusTreeLeafPage<KeyType,ValueType,KeyComparator> *>(p);

    page->Delete(key, comparator_);


  }else{

    /**

      因为BPLUSTREE_TYPE和BPlusTreeInternalPage的explicit template instantiation不同，
      这里的ValueType要明确成page_id_t
    */
    auto page=reinterpret_cast<BPlusTreeInternalPage<KeyType,page_id_t,KeyComparator> *>(p);

    auto index= page->Search(key,comparator_);


    if(index<0)return;
    //page->Temp();
    //page->ToString();

    auto pid=page->GetPointer(index);
    if(pid==INVALID_PAGE_ID)return;

    auto child_guard=bpm_->FetchPageBasic(pid);

     BPlusTreePage* child= child_guard.template AsMut<BPlusTreePage>();

    RemoveHp(key,txn,child,ctx);

    if(child->IsLeafPage()&&child->GetSize()<(child->GetMaxSize())/2){

        //如果右侧有sliding node,先尝试将其合并到child中
        if(index<page->GetSize()-1){

            auto child_guard_next=bpm_->FetchPageBasic(page->GetPointer(index+1));

            BPlusTreePage* child_next= child_guard_next.template AsMut<BPlusTreeLeafPage<KeyType,ValueType,KeyComparator>>();

            if(child->GetSize()+child_next->GetSize()<child->GetMaxSize()){


              return;
            }

            child_guard_next.Drop();
        }

        //如果左侧有sliding node, 尝试将child合并到该node中。
        if(index>0){
  
            auto child_guard_prev=bpm_->FetchPageBasic(page->GetPointer(index-1));

            BPlusTreePage* child_prev= child_guard_prev.template AsMut<BPlusTreeLeafPage<KeyType,ValueType,KeyComparator>>();

            if(child->GetSize()+child_prev->GetSize()<child->GetMaxSize()){


              return;
            }

            child_guard_prev.Drop();

        }

        /**
         *
         * re-distribute
         */

          //可以推断出，左侧(或者右侧)node中entry的数量一定大于half,而且挪掉一个不会使它小于half.

        if(index<page->GetSize()-1){

            auto child_guard_next=bpm_->FetchPageBasic(page->GetPointer(index+1));

            BPlusTreePage* child_next= child_guard_next.template AsMut<BPlusTreeLeafPage<KeyType,ValueType,KeyComparator>>();



            return;

        }


        auto child_guard_prev=bpm_->FetchPageBasic(page->GetPointer(index-1));

        BPlusTreePage* child_prev= child_guard_prev.template AsMut<BPlusTreeLeafPage<KeyType,ValueType,KeyComparator>>();




    }else if((!child->IsLeafPage())&&child->GetSize()<(child->GetMaxSize()+1)/2){

          //如果右侧有sliding node,先尝试将其合并到child中
          if(index<page->GetSize()-1){

            auto child_guard_next=bpm_->FetchPageBasic(page->GetPointer(index+1));

            BPlusTreePage* child_next= child_guard_next.template AsMut<BPlusTreeInternalPage<KeyType,page_id_t ,KeyComparator>>();

            if(child->GetSize()+child_next->GetSize()<=child->GetMaxSize()){


              return;
            }

            child_guard_next.Drop();
          }

          //如果左侧有sliding node, 尝试将child合并到该node中。
          if(index>0){

            auto child_guard_prev=bpm_->FetchPageBasic(page->GetPointer(index-1));

            BPlusTreePage* child_prev= child_guard_prev.template AsMut<BPlusTreeInternalPage<KeyType,page_id_t ,KeyComparator>>();

            if(child->GetSize()+child_prev->GetSize()<=child->GetMaxSize()){


              return;
            }

            child_guard_prev.Drop();

          }

          //re-distribute


          if(index<page->GetSize()-1){

            auto child_guard_next=bpm_->FetchPageBasic(page->GetPointer(index+1));

            BPlusTreePage* child_next= child_guard_next.template AsMut<BPlusTreeLeafPage<KeyType,ValueType,KeyComparator>>();



            return;

          }


          auto child_guard_prev=bpm_->FetchPageBasic(page->GetPointer(index-1));

          BPlusTreePage* child_prev= child_guard_prev.template AsMut<BPlusTreeLeafPage<KeyType,ValueType,KeyComparator>>();


    }

    



  }

} 

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Merge(BPlusTreeInternalPage<KeyType,page_id_t,KeyComparator>* parent,BPlusTreePage* ch1,BPlusTreePage* ch2,int index,const KeyComparator &comparator){



      if(ch1->IsLeafPage()){

         
         auto p1=reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(ch1);
         auto p2=reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(ch2);   
           
         
         B_PLUS_TREE_LEAF_PAGE_TYPE::Move(p1, p2);
           

          parent->Delete(index+1, p1->KeyAt(0),comparator);

      }else{

         auto p1=reinterpret_cast<BPlusTreeInternalPage<KeyType,page_id_t,KeyComparator>  *>(ch1);
         auto p2=reinterpret_cast<BPlusTreeInternalPage<KeyType,page_id_t,KeyComparator>  *>(ch2);


         BPlusTreeInternalPage<KeyType,page_id_t,KeyComparator> ::Move(p1,p2,parent->KeyAt(index+1));

         parent->Delete(index+1,parent->KeyAt(index), comparator);


      }





}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Redistribute(BPlusTreeInternalPage<KeyType,page_id_t,KeyComparator>* parent,BPlusTreePage* ch1,BPlusTreePage* ch2,int index,const KeyComparator &comparator,int type){



  if(ch1->IsLeafPage()){


    auto p1=reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(ch1);
    auto p2=reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(ch2);


    B_PLUS_TREE_LEAF_PAGE_TYPE::Redistribute(p1, p2,type);


    parent->Delete(index+1, p1->KeyAt(0),comparator);

  }else{

    auto p1=reinterpret_cast<BPlusTreeInternalPage<KeyType,page_id_t,KeyComparator>  *>(ch1);
    auto p2=reinterpret_cast<BPlusTreeInternalPage<KeyType,page_id_t,KeyComparator>  *>(ch2);


    BPlusTreeInternalPage<KeyType,page_id_t,KeyComparator> ::Redistribute(p1,p2,parent->KeyAt(index+1));

    parent->Delete(index+1,parent->KeyAt(index), comparator);


  }





}

/*****************************************************************************
 * INDEX ITERATOR
 *****************************************************************************/
/*
 * Input parameter is void, find the leftmost leaf page first, then construct
 * index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Begin() -> INDEXITERATOR_TYPE { return INDEXITERATOR_TYPE(); }

/*
 * Input parameter is low key, find the leaf page that contains the input key
 * first, then construct index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Begin(const KeyType &key) -> INDEXITERATOR_TYPE { return INDEXITERATOR_TYPE(); }

/*
 * Input parameter is void, construct an index iterator representing the end
 * of the key/value pair in the leaf node
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::End() -> INDEXITERATOR_TYPE { return INDEXITERATOR_TYPE(); }

/**
 * @return Page id of the root of this tree
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::GetRootPageId() -> page_id_t {

  BasicPageGuard guard = bpm_->FetchPageBasic(header_page_id_);
  auto header_page = guard.AsMut<BPlusTreeHeaderPage>();

  return header_page->root_page_id_;
}

/*****************************************************************************
 * UTILITIES AND DEBUG
 *****************************************************************************/

/*
 * This method is used for test only
 * Read data from file and insert one by one
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::InsertFromFile(const std::string &file_name, Transaction *txn) {
  int64_t key;
  std::ifstream input(file_name);
  while (input) {
    input >> key;

    KeyType index_key;
    index_key.SetFromInteger(key);
    RID rid(key);
    Insert(index_key, rid, txn);
  }
}
/*
 * This method is used for test only
 * Read data from file and remove one by one
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::RemoveFromFile(const std::string &file_name, Transaction *txn) {
  int64_t key;
  std::ifstream input(file_name);
  while (input) {
    input >> key;
    KeyType index_key;
    index_key.SetFromInteger(key);
    Remove(index_key, txn);
  }
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Print(BufferPoolManager *bpm) {
  auto root_page_id = GetRootPageId();
  auto guard = bpm->FetchPageBasic(root_page_id);
  PrintTree(guard.PageId(), guard.template As<BPlusTreePage>());
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::PrintTree(page_id_t page_id, const BPlusTreePage *page) {
  if (page->IsLeafPage()) {
    auto *leaf = reinterpret_cast<const LeafPage *>(page);
    std::cout << "Leaf Page: " << page_id << "\tNext: " << leaf->GetNextPageId() << std::endl;

    // Print the contents of the leaf page.
    std::cout << "Contents: ";
    for (int i = 0; i < leaf->GetSize(); i++) {
      std::cout << leaf->KeyAt(i);
      if ((i + 1) < leaf->GetSize()) {
        std::cout << ", ";
      }
    }
    std::cout << std::endl;
    std::cout << std::endl;

  } else {
    auto *internal = reinterpret_cast<const InternalPage *>(page);
    std::cout << "Internal Page: " << page_id << std::endl;

    // Print the contents of the internal page.
    std::cout << "Contents: ";
    for (int i = 0; i < internal->GetSize(); i++) {
      std::cout << internal->KeyAt(i) << ": " << internal->ValueAt(i);
      if ((i + 1) < internal->GetSize()) {
        std::cout << ", ";
      }
    }
    std::cout << std::endl;
    std::cout << std::endl;
    for (int i = 0; i < internal->GetSize(); i++) {
      auto guard = bpm_->FetchPageBasic(internal->ValueAt(i));
      PrintTree(guard.PageId(), guard.template As<BPlusTreePage>());
    }
  }
}

/**
 * This method is used for debug only, You don't need to modify
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Draw(BufferPoolManager *bpm, const std::string &outf) {
  if (IsEmpty()) {
    LOG_WARN("Drawing an empty tree");
    return;
  }

  std::ofstream out(outf);
  out << "digraph G {" << std::endl;
  auto root_page_id = GetRootPageId();
  auto guard = bpm->FetchPageBasic(root_page_id);
  ToGraph(guard.PageId(), guard.template As<BPlusTreePage>(), out);
  out << "}" << std::endl;
  out.close();
}

/**
 * This method is used for debug only, You don't need to modify
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::ToGraph(page_id_t page_id, const BPlusTreePage *page, std::ofstream &out) {
  std::string leaf_prefix("LEAF_");
  std::string internal_prefix("INT_");
  if (page->IsLeafPage()) {
    auto *leaf = reinterpret_cast<const LeafPage *>(page);
    // Print node name
    out << leaf_prefix << page_id;
    // Print node properties
    out << "[shape=plain color=green ";
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">P=" << page_id << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">"
        << "max_size=" << leaf->GetMaxSize() << ",min_size=" << leaf->GetMinSize() << ",size=" << leaf->GetSize()
        << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < leaf->GetSize(); i++) {
      out << "<TD>" << leaf->KeyAt(i) << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print Leaf node link if there is a next page
    if (leaf->GetNextPageId() != INVALID_PAGE_ID) {
      out << leaf_prefix << page_id << " -> " << leaf_prefix << leaf->GetNextPageId() << ";\n";
      out << "{rank=same " << leaf_prefix << page_id << " " << leaf_prefix << leaf->GetNextPageId() << "};\n";
    }
  } else {
    auto *inner = reinterpret_cast<const InternalPage *>(page);
    // Print node name
    out << internal_prefix << page_id;
    // Print node properties
    out << "[shape=plain color=pink ";  // why not?
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">P=" << page_id << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">"
        << "max_size=" << inner->GetMaxSize() << ",min_size=" << inner->GetMinSize() << ",size=" << inner->GetSize()
        << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < inner->GetSize(); i++) {
      out << "<TD PORT=\"p" << inner->ValueAt(i) << "\">";
      if (i > 0) {
        out << inner->KeyAt(i);
      } else {
        out << " ";
      }
      out << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print leaves
    for (int i = 0; i < inner->GetSize(); i++) {
      auto child_guard = bpm_->FetchPageBasic(inner->ValueAt(i));
      auto child_page = child_guard.template As<BPlusTreePage>();
      ToGraph(child_guard.PageId(), child_page, out);
      if (i > 0) {
        auto sibling_guard = bpm_->FetchPageBasic(inner->ValueAt(i - 1));
        auto sibling_page = sibling_guard.template As<BPlusTreePage>();
        if (!sibling_page->IsLeafPage() && !child_page->IsLeafPage()) {
          out << "{rank=same " << internal_prefix << sibling_guard.PageId() << " " << internal_prefix
              << child_guard.PageId() << "};\n";
        }
      }
      out << internal_prefix << page_id << ":p" << child_guard.PageId() << " -> ";
      if (child_page->IsLeafPage()) {
        out << leaf_prefix << child_guard.PageId() << ";\n";
      } else {
        out << internal_prefix << child_guard.PageId() << ";\n";
      }
    }
  }
}

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::DrawBPlusTree() -> std::string {
  if (IsEmpty()) {
    return "()";
  }

  PrintableBPlusTree p_root = ToPrintableBPlusTree(GetRootPageId());
  std::ostringstream out_buf;
  p_root.Print(out_buf);

  return out_buf.str();
}

//self-defined
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Real_DrawBPlusTree() -> std::string {


    return "\n"+DrawBPlusTree();
}

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::ToPrintableBPlusTree(page_id_t root_id) -> PrintableBPlusTree {
  auto root_page_guard = bpm_->FetchPageBasic(root_id);
  auto root_page = root_page_guard.template As<BPlusTreePage>();
  PrintableBPlusTree proot;

  if (root_page->IsLeafPage()) {
    auto leaf_page = root_page_guard.template As<LeafPage>();
    proot.keys_ = leaf_page->ToString();
    proot.size_ = proot.keys_.size() + 4;  // 4 more spaces for indent

    return proot;
  }

  // draw internal page
  auto internal_page = root_page_guard.template As<InternalPage>();
  proot.keys_ = internal_page->ToString();
  proot.size_ = 0;
  for (int i = 0; i < internal_page->GetSize(); i++) {
    page_id_t child_id = internal_page->ValueAt(i);
    PrintableBPlusTree child_node = ToPrintableBPlusTree(child_id);
    proot.size_ += child_node.size_;
    proot.children_.push_back(child_node);
  }

  return proot;
}

template class BPlusTree<GenericKey<4>, RID, GenericComparator<4>>;

template class BPlusTree<GenericKey<8>, RID, GenericComparator<8>>;

template class BPlusTree<GenericKey<16>, RID, GenericComparator<16>>;

template class BPlusTree<GenericKey<32>, RID, GenericComparator<32>>;

template class BPlusTree<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
