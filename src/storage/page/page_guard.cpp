#include "storage/page/page_guard.h"
#include "buffer/buffer_pool_manager.h"

namespace bustub {

BasicPageGuard::BasicPageGuard(BasicPageGuard &&that) noexcept {

 bpm_=that.bpm_;
 is_dirty_=that.is_dirty_;
 page_=that.page_;

 that.bpm_= nullptr;
 that.is_dirty_= false;
 that.page_= nullptr;

}

void BasicPageGuard::Drop() {

  if (page_ != nullptr&&bpm_!= nullptr) bpm_->UnpinPage(page_->GetPageId(), is_dirty_);
  bpm_= nullptr;
  is_dirty_=false;
  page_= nullptr

}

auto BasicPageGuard::operator=(BasicPageGuard &&that) noexcept -> BasicPageGuard & {

  BUSTUB_ASSERT(this != &that,"self assignment");


  if (page_ != nullptr) bpm_->UnpinPage(page_->GetPageId(), is_dirty_);

  bpm_=that.bpm_;
  is_dirty_=that.is_dirty_;
  page_=that.page_;

  that.bpm_= nullptr;
  that.is_dirty_= false;
  that.page_= nullptr;


  return *this;


}

BasicPageGuard::~BasicPageGuard(){

    Drop();
};  // NOLINT

ReadPageGuard::ReadPageGuard(ReadPageGuard &&that) noexcept = default;

auto ReadPageGuard::operator=(ReadPageGuard &&that) noexcept -> ReadPageGuard & { return *this; }

void ReadPageGuard::Drop() {}

ReadPageGuard::~ReadPageGuard() {}  // NOLINT

WritePageGuard::WritePageGuard(WritePageGuard &&that) noexcept = default;

auto WritePageGuard::operator=(WritePageGuard &&that) noexcept -> WritePageGuard & { return *this; }

void WritePageGuard::Drop() {}

WritePageGuard::~WritePageGuard() {}  // NOLINT

}  // namespace bustub
