#include "util.h"
#include "heads.h"
#include <algorithm>
#include <vector>

using namespace std;

pthread_t* threads;
pthread_barrier_t bar; //for quicksort
pthread_barrier_t bar1; //for bucket sort

int numB; //number of buckets for bsort
int maximum;
vector<mutex> mut; //vector of mutexes
vector<priority_queue<int, vector<int>, greater<int>>> buckets; //priority queue for b sort to tackle skewness

struct combine_data{
	int start;
    int mid;
	int end;
};

int part(int start, int end){ //to partition the vector for quick sort
    int pivot = lines[end]; //taking last element as pivot same as lab0 quick
    int partition_index = start; //storing first index as part. index

    int temp; //for swapping

    for(int i = start; i < end; i++){
        if(lines[i] <= pivot){ //if line[i] is less than pivot then swap
        //snippet for swap
            temp = lines[i];
            lines[i] = lines[partition_index];
            lines[partition_index] = temp;
            partition_index++; //increment partition index
        }
    }

    temp = lines[end]; //store last value at temp,
    lines[end] = lines[partition_index]; // store int at part index at lines[end]
    lines[partition_index] = temp; //snippet for swap if the cond is not satisfied

    return partition_index; //return the index where it has been partitioned.
}

void *quick_sort(void *input){ //thread function for running in each thread
	struct indices_data *data = (struct indices_data*)input; 
    int start = data->start;
    int end = data->end;
    int tid = data->thread_id;
    if(tid==0){
		clock_gettime(CLOCK_MONOTONIC,&start_time); //clock (timing)
	}

    if(start < end){
        int partition_index = part(start, end); //storing the integer returned from part function

        struct indices_data d[2];
        d[0].start = start;
        d[0].end = partition_index-1;
        d[1].start = partition_index + 1;
        d[1].end = end;
        quick_sort((void *)(&d[0]));
        quick_sort((void *)(&d[1]));
    }

	if(tid==0){
		clock_gettime(CLOCK_MONOTONIC,&end_time);
	}
    return NULL;
}

// function to combine two vectors in sorted order.
// even if we are doing the partition in parallel, the merging is done in sequential method.
// first since different thrreads are doing the part function, the merging cant be done all together.
// so we take one small problem at a time, comb2 function combines 2 vectors in sorted order. pointer is the input.

void *comb2(void *input){
    struct combine_data *data = (struct combine_data*)input;
    int start = data->start;
    int mid = data->mid;
    int end = data->end;

//temp vectors for storing
    vector<int> list_1; 
    vector<int> list_2;

    for(int i = start; i <= mid; i++){
        list_1.push_back(lines[i]);
    }
    for(int i = mid+1; i <= end; i++){
        list_2.push_back(lines[i]);
    }

    size_t current = start, i = 0, j = 0;

    while(i < list_1.size() and j < list_2.size()){
		if(list_1[i]<=list_2[j]){
			lines[current] = list_1[i];
			i++;
		} else {
			lines[current] = list_2[j];
			j++;
		}
		current++;
	}

	while(i < list_1.size()) {
		lines[current] = list_1[i];
		current++;
		i++;
	}

	while(j < list_2.size()) {
		lines[current] = list_2[j];
		current++;
		j++;
	}
    return NULL;
}

// Function to merge multiple subarrays into a sorted order parallelly.
// fullmerge takes in the split size, and number of threads, the for loop runs for log n +1 times 

void fullmerge(size_t split_n, int num_threads){
    int start = 0;
    size_t mid = 0;
    size_t end = 0;
    int tid = 1;
    int prev_tid = 1;
    int times = 0;
    while(times++ <= log(num_threads)+1){    
        start = 0;
        end = 0;
        mid = 0; 
        while(end < lines.size()){
            mid = start + split_n - 1;
            end = mid + split_n;
            if(mid >= lines.size()){
                mid = lines.size()-1;
            }
            if(end >= lines.size()){
                end = lines.size()-1;
            }
            struct combine_data *d = (struct combine_data *) malloc(sizeof(struct combine_data));
            d->start = start;
            d->end = end;
            d->mid = mid;

            int ret = pthread_create(&(threads[tid]), NULL, &comb2, (void *)(d));
            if(ret){
                exit(-1);
            }
            tid += 1;

            if(end >= lines.size() - 1){
                break;
            }
            start = end + 1;
        }
        for(int i = prev_tid; i < tid; i++){
            int ret = pthread_join(threads[i],NULL);
            if(ret){
                exit(-1);
            }
        }
        prev_tid = tid;
        split_n *= 2;
    }
}

// main multithreaded quick sort call. 
//dynamically initialize pthread array, initializing barriers.

void quick_sort_MT(int num_threads, int split_n){
	threads = (pthread_t*)malloc((num_threads)*sizeof(pthread_t)); 
    pthread_barrier_init(&bar, NULL, num_threads); 
	size_t current_chunk = split_n;

	struct indices_data data[num_threads];
    data[0].thread_id = 0;
    data[0].start = 0;
    data[0].end = split_n-1;
    // loop to split the array into num_threads sub arrays for each thread to run
	for(int i = 1; i < num_threads; i++){
		int start = current_chunk;
        int end;
        if(current_chunk + split_n >= lines.size()){
            end = lines.size() - 1;
        }else{
            end = current_chunk + split_n - 1;
        }
        data[i].thread_id = i;
        data[i].start = start;
        data[i].end = end;
        current_chunk += split_n;
	}

    //for loop for worker threads and they all refer and call quick_sort function
	for(int i = 1; i < num_threads; i++){
        int ret = pthread_create(&(threads[i]), NULL, &quick_sort, (void *)(&data[i]));
        if(ret){
            exit(0);
        }
    }

    quick_sort((void *)(&data[0])); //calling quick sort for each sub array

    for(int i=1; i<num_threads; i++){
        int ret = pthread_join(threads[i],NULL); //joining all spawned threads
        if(ret){
            exit(0);
        }
    }
    if(num_threads > 1){
        fullmerge(split_n, num_threads); //this is to merge all the sorted sub arrays in each thread.
    }
    clock_gettime(CLOCK_MONOTONIC,&end_time); //to check the elapsed time
    free(threads);
    pthread_barrier_destroy(&bar);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//BUCKET SORT
///////////////////////////////////////////////////////////////////////////////////////////////////


//bsort to add the elemets to priority queue.
// array of locks (mutexes) are used to lock each priority queue before instering the element.
// locks are used to avoid data races. we store the start index, end index and thread id in the indices data struct.

void *bsort(void *input){
	struct indices_data *data = (struct indices_data*)input;
    int start = data->start;
    int end = data->end;
    int tid = data->thread_id;
    pthread_barrier_wait(&bar1); //to wait for all threads to finish doing inti.
    if(tid==0){
		clock_gettime(CLOCK_MONOTONIC,&start_time);
	}
    for(int i = start; i <= end; i++ ){
        int bnum = floor(numB * lines[i] / maximum);
        if(bnum >= numB){
            bnum = numB - 1;
        }
        mut[bnum].lock(); //locking the queue so that only one thread can access
        buckets[bnum].push(lines[i]);
        mut[bnum].unlock(); //unlocking to let other threads push the elements.
    }

    pthread_barrier_wait(&bar1);
	if(tid==0){
		clock_gettime(CLOCK_MONOTONIC,&end_time); //to get time for displaying.
	}
    return NULL;
}

void bucket_sort_MT(int num_threads, int split_n){
	pthread_t* threads = (pthread_t*)malloc((num_threads)*sizeof(pthread_t)); //to create threads.
    pthread_barrier_init(&bar1, NULL, num_threads);
	size_t current_chunk = split_n;
    numB = ceil(float(log(lines.size()))); //number of buckets defining
    maximum = *std::max_element(lines.begin(), lines.end()); //checkling for maximum element 
    std::vector<std::mutex> list(numB); //making vector of mutexes
    mut.swap(list);
    buckets.resize(numB);//resizing the buckets vector to the number of buckets
	struct indices_data data[num_threads]; //init the data 
    data[0].thread_id = 0;
    data[0].start = 0;
    data[0].end = (split_n)-1; //data[0] is for master thread
    //doing the same for worker threads with a for loop.
	for(int i = 1; i < num_threads; i++){
		int start = current_chunk;
        int end;
        if(current_chunk + split_n >= lines.size()){
            end = lines.size() - 1;
        }else{
            end = current_chunk + split_n - 1;
        }
        data[i].thread_id = i;
        data[i].start = start;
        data[i].end = end;
        current_chunk += split_n;
	}
    //creating and spawning worker threads and they run bsort with data[i] thread number as the parameter.
	for(int i = 1; i < num_threads; i++){
        int ret = pthread_create(&(threads[i]), NULL, &bsort, (void *)(&data[i]));
        if(ret){
            exit(0);
        }
    }
    bsort((void *)(&data[0])); //doing the same for the main thread. 
    for(int i=1; i<num_threads; i++){
        int ret = pthread_join(threads[i],NULL); //joining worker threads to the main thread.
        if(ret){
            exit(0);
        }
    }
    lines.clear(); //clearing the vector
    for(int i = 0; i < numB; i++){
        while(!buckets[i].empty()){
            lines.push_back(buckets[i].top()); //pushing the sorted elements to the original vector in sorted order
            buckets[i].pop();
        }
    }
    clock_gettime(CLOCK_MONOTONIC,&end_time); //getting end time for time calc
    free(threads); //freeing threads
    pthread_barrier_destroy(&bar1); //destroying barrier to avoid memory leaks.
}