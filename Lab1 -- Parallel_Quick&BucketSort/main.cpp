
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <math.h>
#include <getopt.h>
#include <pthread.h>
#include <iterator>
#include <stdlib.h>
#include <unistd.h>
#include "heads.h"
#include "util.h"

using namespace std;

vector<int> lines;
struct timespec start_time, end_time;

void display_time(){
    unsigned long long nanosec;
    nanosec = (end_time.tv_sec-start_time.tv_sec)*1000000000 + (end_time.tv_nsec-start_time.tv_nsec);
    printf("Elapsed (ns): %llu\n",nanosec);
    double secs = ((double)nanosec)/1000000000.0;
    printf("Elapsed (s): %lf\n",secs);
}

int main(int argc, char *argv[]){

    static struct option long_options[] = {
        {"name", no_argument, NULL, 'n'},
        {"alg", required_argument, NULL, 'a'},
        {0, 0, 0, 0}
    };
    bool name = false;
    string input_file_name = "";
    string output_file_name = "";
    int num_threads = 1;
    string Alg_Name = "";

    while (true) {
        int option_index = 0;
        int flag_char = getopt_long(argc, argv, "o:t:", long_options, &option_index);
        if (flag_char == -1) {
          break;
        }

        switch (flag_char) {
            case 'n':
                name = true;
                break;
            case 'a':
                Alg_Name = std::string(optarg);
                break;
            case 'o':
                output_file_name = std::string(optarg);
                break;
            case 't':
                num_threads = stoi(optarg);
                break;
            case '?':
                break;
            default:
                exit(0);
        }
    }

    if (optind < argc) {
        input_file_name = std::string(argv[optind]);
    }

    if(name){
        cout << "Sai Abhishek Aravind\n";
    }

    if(argc > 2){
            ifstream read(input_file_name);
            string n;
            vector<int> line;
            while (read >> n){
            lines.push_back(stoi(n)); }
            int split_n = ceil(float(lines.size()) / num_threads); //splitting vector into sub vectors
            //each thread will get a sub vector to perform the function.
            if(Alg_Name == "forkjoin"){ //checking if the fork join option,
                quick_sort_MT(num_threads, split_n); //calling quick sort multithreaded version
            }
            else if (Alg_Name == "lkbucket"){ //checking if the algname is bucket
                bucket_sort_MT(num_threads, split_n); //calling bucket sort multithreaded version since bsort not defined, I am calling quick sort again.
            }

            ofstream output_file(output_file_name);
            ostream_iterator<signed long long int> output_iterator(output_file, "\n");
            copy(lines.begin(), lines.end(), output_iterator);
            //write_to_file(lines, output_file_name); //function to write the output to the file
            display_time();
        
    }
    return 0;
}

