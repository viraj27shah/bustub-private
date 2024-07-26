# BufferPool

### Why BufferPool :
- OS don't know about how query will use the pages, which page to keep in memory and all those things.
- So dbms will implement that by ownself.
- The buffer pool is an in-memory cache of pages read from disk. It is essentially a large memory region allocated
inside of the database to store pages that are fetched from disk.
- The buffer pool’s region of memory organized as an array of fixed size pages. Each array entry is called a frame.
- When the DBMS requests a page, it is copied from disk into one of the frames of the buffer pool. When a page is requested, the database system can search the buffer pool first. Only if the page is not found, the system fetches
a copy of the page from the disk. Dirty pages are buffered and not written back immediately.

- The buffer pool must maintain certain meta-data in order to be used efficiently and correctly.
- Firstly, the page table is an in-memory hash table that keeps track of pages that are currently in memory. It maps
page ids to frame locations in the buffer pool. Since the order of pages in the buffer pool does not necessarily reflect
the order on the disk, this extra indirection layer allows for the identification of page locations in the pool.
- Note: The page table is not to be confused with the page directory, which is the mapping from page ids to page
locations in database files. All changes to the page directory must be recorded on disk to allow the DBMS to find on
restart.
- The page table also maintains additional meta-data per page, a dirty-flag and a pin/reference counter.
- The dirty-flag is set by a thread whenever it modifies a page. This indicates to storage manager that the page must
be written back to disk.
- The pin/reference Counter tracks the number of threads that are currently accessing that page (either reading or
modifying it). A thread has to increment the counter before they access the page. If a page’s pin count is greater
than zero, then the storage manager is not allowed to evict that page from memory. Pinning does not prevent other
transactions from accessing the page concurrently.


- Database Management Systems (DBMS) create their own buffer pool management to efficiently handle the caching of frequently accessed data in memory.
- DBMS can tailor buffer pool management to the specific access patterns and workload characteristics of database operations, such as read-heavy or write-heavy workloads.
- By keeping frequently accessed data in memory, the DBMS reduces disk I/O operations, leading to faster query execution and improved response times.

## Task #1 - LRU-K Replacement Policy
- This component is responsible for tracking page usage in the buffer pool. You will implement a new class called LRUKReplacer in src/include/buffer/lru_k_replacer.h and its corresponding implementation file in src/buffer/lru_k_replacer.cpp. Note that LRUKReplacer is a stand-alone class and is not related to any of the other Replacer classes. You are expected to implement only the LRU-K replacement policy. You don't have to implement LRU or a clock replacement policy, even if there is a corresponding file for it.

- The LRU-K algorithm evicts a frame whose backward k-distance is maximum of all frames in the replacer. Backward k-distance is computed as the difference in time between current timestamp and the timestamp of kth previous access. A frame with fewer than k historical accesses is given +inf as its backward k-distance. When multiple frames have +inf backward k-distance, the replacer evicts the frame with the earliest overall timestamp (i.e., the frame whose least-recent recorded access is the overall least recent access, overall, out of all frames).

- The maximum size for the LRUKReplacer is the same as the size of the buffer pool since it contains placeholders for all of the frames in the BufferPoolManager. However, at any given moment, not all the frames in the replacer are considered to be evictable. The size of LRUKReplacer is represented by the number of evictable frames. The LRUKReplacer is initialized to have no frames in it. Then, only when a frame is marked as evictable, replacer's size will increase.

- You will need to implement the LRU-K policy discussed in this course. You will need to implement the following methods as defined in the header file (src/include/buffer/lru_k_replacer.h) and in the source file (src/buffer/lru_k_replacer.cpp):

- Evict(frame_id_t* frame_id) : Evict the frame with largest backward k-distance compared to all other evictable frames being tracked by the Replacer. Store the frame id in the output parameter and return True. If there are no evictable frames return False.
- RecordAccess(frame_id_t frame_id) : Record that given frame id is accessed at current timestamp. This method should be called after a page is pinned in the BufferPoolManager.
- Remove(frame_id_t frame_id) : Clear all access history associated with a frame. This method should be called only when a page is deleted in the BufferPoolManager.
- SetEvictable(frame_id_t frame_id, bool set_evictable) : This method controls whether a frame is evictable or not. It also controls LRUKReplacer's size. You'll know when to call this function when you implement the BufferPoolManager. To be specific, when pin count of a page reaches 0, its corresponding frame is marked evictable and replacer's size is incremented.
- Size() : This method returns the number of evictable frames that are currently in the LRUKReplacer.
- The implementation details are up to you. You are allowed to use built-in STL containers. You may assume that you will not run out of memory, but you must make sure that your implementation is thread-safe.

### Overview 

### Why
- Better than LRU+CLOCK
- LRU + CLOCK replacement policies are
susceptible to sequential flooding.
- A query performs a sequential scan that reads every page.
- This pollutes the buffer pool with pages that are read
once and then never again.
- In OLAP workloads, the most recently used page is often
the best page to evict.
- LRU + CLOCK only tracks when a page was last
accessed, but not how often a page is accessed.
- REFER TO slides
- LRU-K (Least Recently Used-K) is an advanced cache replacement policy that extends the traditional LRU algorithm by considering the history of access patterns. 
- LRU-K tracks the last K references to a page, providing a more comprehensive understanding of access patterns and improving the prediction of which pages will be accessed in the future.
- By considering multiple recent accesses (K), LRU-K can adapt to different types of workloads, including those with bursty or periodic access patterns.


### Files : 
- Have lool in this files for detail description:
- src/buffer/lru_k_replacer.cpp
- src/include/buffer/lru_k_replacer.h

### Data Structure :

- Class LRUKReplacer : 
    - std::unordered_map<frame_id_t, LRUKNode*> node_store_;
    - size_t replacer_size_;                                 // which tracks the number of evictable frames.
    - size_t k_;
  
    - MinHeap min_heap_obj_;                                    // Keep track of what to evict next
    // storing LRUKNode pointers in it.
    - size_t total_num_frames;


- Storing frame_id -> LRUKNode in node_store_ map.
- replacer_size will keep the info of evictable frames.
- min heap will keep track of evictable frames, oldest kth frame will be at top.

- Class LRUKNode :
    - std::list<size_t> history_; -> latest time stamp will be at front. and old one at the end, initialize with k 0 values.
    - size_t k_; -> k lookup
    - frame_id_t fid_; -> frame id
    - bool is_evictable_{false};                  // false means it will not present in min heap and you can not evict it
    - size_t index_in_heap_arr_; -> mapping of index of min heap array at which index this node is ptesent.

- Class MinHeap : 
    - arr_ele -> array of LRUKNode pointers, It will only store the evictable nodes only


### Functions :

- Evict : Oldest kth access frame will be evected.
- RecordAccess : First we will check wheather frame is present in map or not if not then we will create a new LRUKnode else just update the current node.
- SetEvictable : change the state of evicatble.
- Remove : remove the evicatble frame from heap and map.


## Task #3 - Buffer Pool Manager

- Next, implement the buffer pool manager (BufferPoolManager). The BufferPoolManager is responsible for fetching database pages from disk with the DiskScheduler and storing them in memory. The BufferPoolManager can also schedule writes of dirty pages out to disk when it is either explicitly instructed to do so or when it needs to evict a page to make space for a new page.

- To make sure that your implementation works correctly with the rest of the system, we will provide you with some functions already filled in. You will also not need to implement the code that actually reads and writes data to disk (this is called the DiskManager in our implementation). We will provide that functionality. You do, however, need to implement the DiskScheduler to process disk requests and dispatch them to the DiskManager (this is Task #2).

- All in-memory pages in the system are represented by Page objects. The BufferPoolManager does not need to understand the contents of these pages. But it is important for you as the system developer to understand that Page objects are just containers for memory in the buffer pool and thus are not specific to a unique page. That is, each Page object contains a block of memory that the DiskManager will use as a location to copy the contents of a physical page that it reads from disk. The BufferPoolManager will reuse the same Page object to store data as it moves back and forth to disk. This means that the same Page object may contain a different physical page throughout the life of the system. The Page object's identifer (page_id) keeps track of what physical page it contains; if a Page object does not contain a physical page, then its page_id must be set to INVALID_PAGE_ID.

- Each Page object also maintains a counter for the number of threads that have "pinned" that page. Your BufferPoolManager is not allowed to free a Page that is pinned. Each Page object also keeps track of whether it is dirty or not. It is your job to record whether a page was modified before it is unpinned. Your BufferPoolManager must write the contents of a dirty Page back to disk before that object can be reused.

- Your BufferPoolManager implementation will use the LRUKReplacer and DiskScheduler classes that you created in the previous steps of this assignment. The LRUKReplacer will keep track of when Page objects are accessed so that it can decide which one to evict when it must free a frame to make room for copying a new physical page from disk. When mapping page_id to frame_id in the BufferPoolManager, again be warned that STL containers are not thread-safe. The DiskScheduler will schedule writes and reads to disk on the DiskManager.

- You will need to implement the following functions defined in the header file (src/include/buffer/buffer_pool_manager.h) and in the source file (src/buffer/buffer_pool_manager.cpp):

- FetchPage(page_id_t page_id)
- UnpinPage(page_id_t page_id, bool is_dirty)
- FlushPage(page_id_t page_id)
- NewPage(page_id_t* page_id)
- DeletePage(page_id_t page_id)
- FlushAllPages()
- For FetchPage, you should return nullptr if no page is available in the free list and all other pages are currently pinned. FlushPage should flush a page regardless of its pin status.

- For UnpinPage, the is_dirty parameter keeps track of whether a page was modified while it was pinned.

- The AllocatePage private method provides the BufferPoolManager a unique new page id when you want to create a new page in NewPage(). On the other hand, the DeallocatePage() method is a no-op that imitates freeing a page on the disk and you should call this in your DeletePage() implementation.

- You do not need to make your buffer pool manager super efficient -- holding the buffer pool manager lock from the start to the end in each public-facing buffer pool manager function should be enough. However, you do need to ensure your buffer pool manager has reasonable performance, otherwise there will be problems in future projects. You can compare your benchmark result (QPS.1 and QPS.2) with other students and see if your implementation is too slow.

- Please refer to the header files (lru_k_replacer.h, disk_scheduler.h, buffer_pool_manager.h) for more detailed specs and documentations.

### Tasks
- The BufferPoolManager is responsible for fetching database pages from disk with the DiskScheduler and storing them in memory. 
- The BufferPoolManager can also schedule writes of dirty pages out to disk when it is either **explicitly instructed** to do so or when it needs to **evict a page** to make space for a new page.
- All in-memory pages in the system are represented by Page objects. 
- The Page object's identifer (page_id) keeps track of what physical page it contains; if a Page object does not contain a physical page, then its page_id must be set to INVALID_PAGE_ID.
- Each Page object also maintains a counter for the number of threads that have "pinned" that page.
- Your BufferPoolManager is **not allowed to free a Page that is pinned**.
- Each Page object also keeps track of **whether it is dirty or not**. It is your job to record whether a page was modified before it is unpinned.
- Your BufferPoolManager must write the contents of a dirty Page back to disk before that object can be **reused**.
- LRUKReplacer will keep track of when Page objects are accessed so that it can decide which one to evict when it must free a frame to make room for copying a new physical page from disk.
- When mapping page_id to frame_id in the BufferPoolManager, again be warned that STL containers are not thread-safe. 
- Use mutexes or other locking mechanisms when accessing shared containers from multiple threads.






- LRU K Replacer has nothing to do with pages
- It only has info about frames
- That How amny times and when this frame is accessed irrespective what data is inside
- It only maintains frameid-> LRUKnode mapping
- Bufferpool will decide when to evict by seeing free list -> which contains frame id (frame id which not contain any page)
- It will ask lru K replacer which frame to evict


- frame_id indicates the array index of page array

- In LRU K Replacer replace_size_ will keep track how many frere frames(is_evictable with true) is available to evict
- Only those LRU K nodes will be present in min heap which is set evictable with true.



### HOW TO RUN
### Full latency check
```SHELL
mkdir cmake-build-relwithdebinfo
cd cmake-build-relwithdebinfo
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j`nproc` bpm-bench
./bin/bustub-bpm-bench --duration 5000 --latency 1
```

### Normal Functionality Check
- LRUKReplacer: test/buffer/lru_k_replacer_test.cpp
- DiskScheduler: test/storage/disk_scheduler_test.cpp
- BufferPoolManager: test/buffer/buffer_pool_manager_test.cpp
```SHELL
$ make lru_k_replacer_test -j$(nproc)
$ ./test/lru_k_replacer_test
```