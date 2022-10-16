#include <stdlib.h>
#include <vector>
#include <mutex>
#include <queue>

using namespace std;


void quick_sort_MT(int num_threads, int split_size);
void *quick_sort(void *data);
int part(int start, int end);
void *comb2(void *input);
void fullmerge(size_t split_size, int num_threads);

void bucket_sort_MT(int num_threads, int split_size);
void *bsort(void *data);
