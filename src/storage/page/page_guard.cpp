#include "storage/page/page_guard.h"
#include "buffer/buffer_pool_manager.h"

namespace bustub {

BasicPageGuard::BasicPageGuard(BasicPageGuard &&that) noexcept {
  LOG("BasicPageGuard::BasicPageGuard",that.page_== nullptr? -1:that.PageId()) ;
 bpm_=that.bpm_;
 is_dirty_=that.is_dirty_;
 page_=that.page_;
 dropped= that.dropped;

 that.bpm_= nullptr;
 that.is_dirty_= false;
 that.page_= nullptr;
 that.dropped=false;

}

void BasicPageGuard::Drop() {

  LOG("BasicPageGuard::Drop",page_== nullptr? -1:PageId()) ;
  if(dropped)return;
  if (page_ != nullptr&&bpm_!= nullptr) bpm_->UnpinPage(page_->GetPageId(), is_dirty_);
  bpm_= nullptr;
  is_dirty_=false;
  page_= nullptr;

  dropped=true;

}

auto BasicPageGuard::operator=(BasicPageGuard &&that) noexcept -> BasicPageGuard & {
  LOG("BasicPageGuard::operator=",that.page_== nullptr? -1:that.PageId()) ;
  BUSTUB_ASSERT(this != &that,"self assignment");


  //if (page_ != nullptr) bpm_->UnpinPage(page_->GetPageId(), is_dirty_);
  Drop();//must drop the existing resources. or will cause deadlock in write.
  bpm_=that.bpm_;
  is_dirty_=that.is_dirty_;
  page_=that.page_;
  dropped=that.dropped;

  that.bpm_= nullptr;
  that.is_dirty_= false;
  that.page_= nullptr;
  that.dropped=false;

  return *this;


}

BasicPageGuard::~BasicPageGuard(){
    LOG("BasicPageGuard::~BasicPageGuard",page_== nullptr? -1:page_->GetPageId()) ;
    if(!dropped)Drop();
};  // NOLINT

//==============================================================

ReadPageGuard::ReadPageGuard(ReadPageGuard &&that) noexcept
    :guard_(std::move(that.guard_))
{
  LOG("ReadPageGuard::ReadPageGuard",that.guard_.page_== nullptr? -1:that.PageId()) ;

  dropped=that.dropped;

  that.dropped=false;
}

auto ReadPageGuard::operator=(ReadPageGuard &&that) noexcept -> ReadPageGuard & {
  LOG("ReadPageGuard::operator=",that.guard_.page_== nullptr? -1:that.PageId()) ;

  if (this != &that) {

    /**
     *
     * 根据page_guard.cpp的说法,this本身是hold某个page或者bpm的，
     * 所以要先drop掉,否则就deadlock了
     */
    Drop();
    guard_ = std::move(that.guard_);
    // Additional resource transfers or logic specific to ReadPageGuard
    // can be added here if needed.
    dropped=that.dropped;
    that.dropped=false;
  }
  return *this;

}

void ReadPageGuard::Drop() {
  LOG("ReadPageGuard::Drop",guard_.page_== nullptr? -1:PageId()) ;

  /**
   *
   * TODO:drop的顺序有待商榷。
   *
   * 个人观点，如果先unpin(在guard_.Drop()中)，然后unlatch page，这时可能会出现问题。
   * 因为unpin后,如果说page的pin变成0,那么page的内容可能就会被替换掉(准确来说是frame)，
   * 但此时page的锁还没有被释放。等到this释放latch后，另一个线程就能够进入这个page了(准确说是frame)
   * 从运行来看，这样做似乎也没什么问题。
   *
   * TODO:我的方法是先unlatch再unpin。似乎也是没问题的。如果后续出错，可以先改这里试一下。
   */

  if(dropped)return;
  if(guard_.page_!= nullptr)guard_.page_->RUnlatch();
  guard_.Drop();

  dropped=true;

}

ReadPageGuard::~ReadPageGuard() {

  LOG("ReadPageGuard::~ReadPageGuard",guard_.page_== nullptr? -1:PageId()) ;


  if(dropped)return;

  Drop();

}  // NOLINT

WritePageGuard::WritePageGuard(WritePageGuard &&that) noexcept

    :guard_(std::move(that.guard_))
{

  LOG("WritePageGuard::WritePageGuard",that.guard_.page_== nullptr? -1:that.PageId()) ;

  dropped=that.dropped;

  that.dropped=false;
};

auto WritePageGuard::operator=(WritePageGuard &&that) noexcept -> WritePageGuard & {

  LOG("WritePageGuard::operator=",that.guard_.page_== nullptr? -1:that.PageId()) ;

  if (this != &that) {
    Drop();//must drop the existing resources. or will cause deadlock.
    guard_ = std::move(that.guard_);
    // Additional resource transfers or logic specific to ReadPageGuard
    // can be added here if needed.
    dropped=that.dropped;
    that.dropped=false;
  }
  return *this;


}

void WritePageGuard::Drop() {

  LOG("WritePageGuard::Drop",guard_.page_== nullptr? -1:PageId()) ;

  if(dropped)return;
  if(guard_.page_!= nullptr)guard_.page_->WUnlatch();
  guard_.Drop();

  dropped=true;

}

WritePageGuard::~WritePageGuard() {

  LOG("WritePageGuard::~WritePageGuard",guard_.page_== nullptr? -1:PageId()) ;

  if(!dropped)Drop();
}  // NOLINT

}  // namespace bustub
