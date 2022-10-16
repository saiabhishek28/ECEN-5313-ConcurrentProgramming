
# LAB 1 - Parallel Quick and Bucket Sort

To implement parallization strat to Quick and Bucket sort using Barriers and Fork Join design methodology.




## Deployment

To deploy this project run

```bash
  make
```

```bash
  ./mysort <inputfile_Name> -o <outputfile_Name> -t <NUM_THREADS> --alg=<forkjoin or lkbucket>
```

#### Example: 

Running Quick Sort on myfile.txt with 5 threads:

```bash
  ./mysort myfile.txt -o outputfile.txt -t 5 --alg=forkjoin
```
## Appendix

#### Quick Sort:

The approach for this is to divide the vector into sub vectors and pass each subvector to a single thread. Quick sort uses divide and conquer approach as discussed in the lab0. Same as lab0, the last element is selected as the pivot and in the first iteration elements smaller than pivot are placed in the right side and larger elements to the left side, then the same function is called recursively and the subvector is partitioned at the pivot element. The partitioned vectors again go through the same process and they select the last element as the pivot. This process is repeated until every subvector has only one element. Then we are going to merge the subvectors to get the sorted array. The above process is happening CONCURRENTLY in mulitple threads. Now the complication arises when we need to merge the subarrays to get the sorted vector. Merging is done in 2 stages. First 2 sub vectors are merged and sorted, for this the comb2 function is used. Then the previously merged (sorted) sub vector is merged with the next merged subvector from other threads and is sorted. These two processes happen one after the other CONCURRENTLY and each merge operation takes place in a single thread. Finally when all the merged sub vectors are finally merged we get the sorted vector.


#### Bucket Sort:

The approach for this algorithm is to insert the elements of the vector into separate buckets such that the buckets are sorted (ordered). Then once all the elements have been added to respective buckets and are sorted, we merge the buckets to form a fully sorted vector. Each bucket is handled by a separate thread and a vector (array) of locks are used to write the values into the buckets. Priority queue is used because we want the buckets to be ordered and sorted. Each bucket is a priority queue, so we make an array of "priority queues". Same as the Quick Sort implementation, the vector is divided into sub vectors based on the number of threads, and each thread will get a sub vector to parse and add the elements into respective buckets. At any instance only one thread can access the priority queue for an element type so we are using locks. Before writing the element to the PQueue we first lock the respective queue add the element and then unlock it so that other threads can try to obtain the lock. This way we are preventing any and all DATA RACES when it comes to writing the elements to the PQueue. Then once each element is added to the respective priority queue, the final step is to merge all the elements in the order of the priority queue array thereby giving us the final sorted vector.



