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