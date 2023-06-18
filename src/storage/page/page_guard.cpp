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


  if (page_ != nullptr) bpm_->UnpinPage(page_->GetPageId(), is_dirty_);

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
