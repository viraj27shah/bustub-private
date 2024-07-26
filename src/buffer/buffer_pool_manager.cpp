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
    : pool_size_(pool_size), disk_scheduler_(std::make_unique<DiskScheduler>(disk_manager)), log_manager_(log_manager) {
  // TODO(students): remove this line after you have implemented the buffer pool manager
  // throw NotImplementedException(
  //     "BufferPoolManager is not implemented yet. If you have finished implementing BPM, please remove the throw "
  //     "exception line in `buffer_pool_manager.cpp`.");

  // we allocate a consecutive memory space for the buffer pool
  pages_ = new Page[pool_size_];
  replacer_ = std::make_unique<LRUKReplacer>(pool_size, replacer_k);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }
}

BufferPoolManager::~BufferPoolManager() { delete[] pages_; }

void BufferPoolManager::ActOnDirtyBit(frame_id_t frame_id)
{
  if(pages_[frame_id].GetPageId() == INVALID_PAGE_ID)
    return;
  // Dirty bit is set, so write data back on disk of this page
  if(pages_[frame_id].IsDirty())
  {
    DiskRequest r;
    r.is_write_= true;
    r.page_id_ = pages_[frame_id].GetPageId();
    r.data_ = pages_[frame_id].GetData();
    
    auto promise1 = disk_scheduler_->CreatePromise();
    auto future1 = promise1.get_future();

    r.callback_ = std::move(promise1);

    disk_scheduler_->Schedule(std::move(r));

    if(future1.get() == false)
    {
      BUSTUB_ASSERT(false,"Something went wrong while eriting dirty page back to disk");
    }
    pages_[frame_id].SetDirtyBit(false);
  }
  return;
}

/**
 * check frame validity
 * check pin count
 * act on dirty bit status
 * new page id
 * entry in page_table_
 * resent info of page(old page stored in frame id location in pages_ array)
 * record access and setevicatble false
 */
auto BufferPoolManager::ProceesOfNewPage(frame_id_t frame_id) -> page_id_t
{
  // check for frame_id is valid or not
  BUSTUB_ASSERT(validityOfFrame(frame_id),"Frame id return by EVICT is not valid");

  // Check pin count is not more than 0
  BUSTUB_ASSERT(pages_[frame_id].GetPinCount() == 0,"Pin count is not 0 so you can not evict this");

  // check dirty bit and act accordingly
  ActOnDirtyBit(frame_id);

  // GET LOCK BEFORE CALLING THIS
  page_id_t new_page_id = AllocatePage();

  // *page_id = new_page_id;

  // Remove last entry of page id from page_table
  page_table_.erase(pages_[frame_id].GetPageId());

  // Do entry in page-<iud to fram_id 
  page_table_[new_page_id] = frame_id;

  // set new info in new page
  pages_[frame_id].SetPinCount(0);
  pages_[frame_id].SetPageId(new_page_id);
  pages_[frame_id].ResetMemoryData();
  pages_[frame_id].IncPinCount();
  pages_[frame_id].SetDirtyBit(false);

  // Do entry of history and make it inevictable.
  replacer_->RecordAccess(frame_id);
  replacer_->SetEvictable(frame_id,false);

  return new_page_id;
}

// For Fetching a page
void BufferPoolManager::ProceesOfFetchPage(frame_id_t frame_id,page_id_t page_id)
{
  // check for frame_id is valid or not
  BUSTUB_ASSERT(validityOfFrame(frame_id),"Frame id return by EVICT is not valid");

  // Check pin count is not more than 0
  BUSTUB_ASSERT(pages_[frame_id].GetPinCount() == 0,"Pin count is not 0 so you can not evict this");

  // check dirty bit and act accordingly
  ActOnDirtyBit(frame_id);

  // Reading a page from a disk
  DiskRequest r;
  r.is_write_= false;
  r.page_id_ = page_id;
  r.data_ = pages_[frame_id].data_;
  auto promise1 = disk_scheduler_->CreatePromise();
  auto future1 = promise1.get_future();

  r.callback_ = std::move(promise1);

  disk_scheduler_->Schedule(std::move(r));

  if(future1.get() == false)
  {
    BUSTUB_ASSERT(false,"Something went wrong while eriting dirty page back to disk");
  }

  // Remove last entry of page id from page_table
  page_table_.erase(pages_[frame_id].GetPageId());

  page_table_[page_id] = frame_id;

  // set new info in new page
  pages_[frame_id].SetPinCount(0);
  pages_[frame_id].SetPageId(page_id);
  pages_[frame_id].IncPinCount();
  pages_[frame_id].SetDirtyBit(false);

  // Do entry of history and make it inevictable.
  replacer_->RecordAccess(frame_id);
  replacer_->SetEvictable(frame_id,false);

  return;
}

auto BufferPoolManager::NewPage(page_id_t *page_id) -> Page * 
{
  // set output in page_id pointer
  // Already page array contains objects of pages
  // so get the frame id - which denotes the array index on pages array - get it from free list first
  // if no frame available then  try to evict
  //  get the page id from allocate page()
  // check for dirty bit, You also need to reset the memory and metadata for the new page.
  // Remember to "Pin" the frame by calling replacer.SetEvictable(frame_id, false)
  //  Also, remember to record the access history of the frame in the replacer for the lru-k algorithm to work.
  // return pageid
  //  if no frame available then return null ptr

  std::lock_guard<std::mutex> lock(latch_);

  // Just an initialization
  frame_id_t frame_id = pool_size_;
  
  // No free page/frame is available
  if(free_list_.size() == 0)
  {
    // Evicatble frame is found , it will return true if any evictable frame found, then we can simply use that frame id
    if(replacer_->Evict(&frame_id))
    {
      
    }
    // No evicatble frames are present
    else
    {
      return nullptr;
    }
  }
  // free frame is available
  else
  {
    frame_id = free_list_.front();
    free_list_.pop_front();
  }
  page_id_t new_page_id = ProceesOfNewPage(frame_id);
  *page_id = new_page_id;
  return &(pages_[frame_id]);
}

auto BufferPoolManager::FetchPage(page_id_t page_id, [[maybe_unused]] AccessType access_type) -> Page * {
  
  std::lock_guard<std::mutex> lock(latch_);
  
  frame_id_t frame_id = pool_size_;
  // First look in the buffer pool 
  if(page_table_.find(page_id)!=page_table_.end())
  {
    frame_id = page_table_[page_id];
    // check for frame_id is valid or not
    BUSTUB_ASSERT(validityOfFrame(frame_id),"Frame id return by EVICT is not valid");
    pages_[frame_id].IncPinCount();

    // Do entry of history and make it inevictable.
    replacer_->RecordAccess(frame_id);
    replacer_->SetEvictable(frame_id,false);
  }
  else
  {
    // No free page/frame is available
    if(free_list_.size() == 0)
    {
      // Evicatble frame is found , it will return true if any evictable frame found, then we can simply use that frame id
      if(replacer_->Evict(&frame_id))
      {
        
      }
      // No evicatble frames are present
      else
      {
        return nullptr;
      }
    }
    // free frame is available
    else
    {
      frame_id = free_list_.front();
      free_list_.pop_front();

    }
    ProceesOfFetchPage(frame_id,page_id);
  }
  return &(pages_[frame_id]);
}

auto BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty, [[maybe_unused]] AccessType access_type) -> bool {
  
  std::lock_guard<std::mutex> lock(latch_);

  // Either page is not present in bufferpool or pincount of page is already 0
  if(page_table_.find(page_id) == page_table_.end() || pages_[page_table_[page_id]].GetPinCount() <= 0)
  {
    return false;
  }
  // Otherwise
  else
  {
    frame_id_t frame_id = page_table_[page_id];
    pages_[frame_id].DecPinCount();
    // Pincount is 0
    if(pages_[frame_id].GetPinCount() == 0)
    {
      replacer_->SetEvictable(frame_id,true);
    }
    // Set dirty bit
    
    pages_[frame_id].SetDirtyBit(is_dirty);
    
    return true;
  }
  return false;
}

auto BufferPoolManager::FlushPage(page_id_t page_id) -> bool 
{ 
  std::lock_guard<std::mutex> lock(latch_);

  // Either page is not present in bufferpool or invalid page id
  if(page_id == INVALID_PAGE_ID || page_table_.find(page_id) == page_table_.end())
  {
    return false;
  }
  // Otherwise
  else
  {
    frame_id_t frame_id = page_table_[page_id];
    // Setting dirty bit to true so that ActOnDirtyBit function can be called.
    pages_[frame_id].SetDirtyBit(true);
    ActOnDirtyBit(frame_id);
    return true;
  }
  return false; 
}

void BufferPoolManager::FlushAllPages() 
{
  std::lock_guard<std::mutex> lock(latch_);

  for(auto page_table_entry : page_table_)
  {
    page_id_t page_id = page_table_entry.first;
    frame_id_t frame_id = page_table_entry.second;
    // check for frame_id is valid or not
    BUSTUB_ASSERT(validityOfFrame(frame_id),"Frame id return by EVICT is not valid");
    
    // if invalid page id
    if(page_id == INVALID_PAGE_ID)
      continue;
    // Otherwise
    else
    {
      // Setting dirty bit to true so that ActOnDirtyBit function can be called.
      pages_[frame_id].SetDirtyBit(true);
      ActOnDirtyBit(frame_id);
    }
  }
  return;
}

auto BufferPoolManager::DeletePage(page_id_t page_id) -> bool 
{ 
  std::lock_guard<std::mutex> lock(latch_);

  // Either page is not present in bufferpool or invalid page id
  if(page_id == INVALID_PAGE_ID || page_table_.find(page_id) == page_table_.end())
  {
    return false;
  }
  else
  {
    frame_id_t frame_id = page_table_[page_id];
    // Can't delete if pincount is more
    if(pages_[frame_id].GetPinCount() > 0)
    {
      return false;
    }
    else
    {
      // Write data if dirty bit is set
      ActOnDirtyBit(frame_id);
      
      // remove entry of page id from page table
      page_table_.erase(page_id);

      // Remove function of replacer
      replacer_->Remove(frame_id);

      // Do entry in free list
      free_list_.push_back(frame_id);

      // Reset page data
      pages_[frame_id].ResetMemoryData();
      pages_[frame_id].SetPageId(INVALID_PAGE_ID);
      pages_[frame_id].SetPinCount(0);
      pages_[frame_id].SetDirtyBit(false);

      // Call Deallocate() function
      DeallocatePage(page_id);
    }
  } 
  return false; 
}

auto BufferPoolManager::AllocatePage() -> page_id_t { return next_page_id_++; }

auto BufferPoolManager::FetchPageBasic(page_id_t page_id) -> BasicPageGuard { return {this, nullptr}; }

auto BufferPoolManager::FetchPageRead(page_id_t page_id) -> ReadPageGuard { return {this, nullptr}; }

auto BufferPoolManager::FetchPageWrite(page_id_t page_id) -> WritePageGuard { return {this, nullptr}; }

auto BufferPoolManager::NewPageGuarded(page_id_t *page_id) -> BasicPageGuard { return {this, nullptr}; }

bool BufferPoolManager::validityOfFrame(frame_id_t frame_id)
{ 
    if(frame_id <0 || frame_id >= static_cast<frame_id_t>(pool_size_))
        return false; 
    else    
        return true;
}

}  // namespace bustub
