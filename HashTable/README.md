# Open-Addressed Hashtable Operations on a Dynamic Array

---

This mini-project came about from a university assignment teaching basic OOP. Part of the assignment required being able to check if a passenger existed on a plane and forced us to use a simple dynamic array. I took the opportunity to learn from scratch how to get O(1) behaviour using a hash from scratch rather than a sequential search.
It's an example of my experience with lower-level data structures and raw bit manipulation. I attempt to make use of another structure I designed: DynamicBitset which was an attempt (not a wholly successful one) to emulate an 'std::bitset<N>' but with dynamic behaviour to track which buckets are used up. 
The actual assignment used the hashing, probing, and maintaining 2^n sizes to avoid collisions used in HashTable.h. The O(2^n) space growth is problematic for larger datasets but I decided it was light enough for regular plane capacities. Testing using 'std::chrono' showed that 500,000 random insertions and deletions could be performed in around 8 seconds (tests on a sequential search implementation were not able to finish but would probably take several days for the same task).
