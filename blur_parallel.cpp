#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <pthread.h>
#include <array>
#include <cstring>
#include <unistd.h>
// #include <pthread.h>
#include <chrono>

#include "./qdbmp.hpp"

using namespace std;

// defining a struct to pass multiple arguments into each of the threads
struct thread_args {
    BitMap* image;
    BitMap* blurred;
    int box_size;
    int start_row;
    int end_row;
};

// function to be executed by each thread
void* threadmain(void* ars){
    thread_args* arguments = static_cast<thread_args*>(ars);
    BitMap* image = arguments->image;
    BitMap* blurred = arguments->blurred;
    int box_size = arguments->box_size;
    int start_row = arguments->start_row;
    int end_row = arguments->end_row;

    unsigned width = image->width();
    unsigned height = image->height();

    // Loop through each pixel and turn into blurred
    for (int y = start_row; y <= end_row; y++){
        for (int x = 0; x < width; x++){
            // get the pixel
            // RGB color = image->get_pixel(x, y);
            int red_sum = 0;
            int green_sum = 0;
            int blue_sum = 0;
            int count = 0;

            // determining the neighbours as we have to loop through each of the pixels 
            for (int i = -box_size; i <= box_size; i++){
                for (int j = -box_size; j <= box_size; j++){
                    int new_x = x + i;
                    int new_y = y + j;

                    if (new_x >= 0 && new_x < width && new_y >= 0 && new_y < height){
                        RGB neighbour_color = image->get_pixel(new_x, new_y);
                        red_sum += neighbour_color.red;
                        green_sum += neighbour_color.green;
                        blue_sum += neighbour_color.blue;
                        count++;
                    }
                }
            }

            // calculating the average
            if (count>0){
            RGB blurred_color{
                static_cast<UCHAR>(red_sum / count),
                static_cast<UCHAR>(green_sum / count),
                static_cast<UCHAR>(blue_sum / count)
            };

            // setting the blurred pixel
            blurred->set_pixel(x, y, blurred_color);
        }
    }
    
    }
    return nullptr; // return type is a pointer
}



int main(int argc, char* argv[]) {
 // adding a thread argument
 if (argc != 5){
    cerr << "Usage: " << argv[0] << " <input file> <output file> <box size> <number of threads>" << endl;
    return EXIT_FAILURE;
 }

 const int thread_count = stoi(argv[4]);
 const int box_size = stoi(argv[3]);

 if (thread_count <= 0){
   cerr << "ERROR: Number of threads must be positive." << endl;
   return EXIT_FAILURE;
 }
 if (box_size<=0){
   cerr << "ERROR: Box size must be positive." << endl;
   return EXIT_FAILURE;
 }

 string input_fname{argv[1]};
 string output_fname{argv[2]};

 BitMap image(input_fname);
    if (image.check_error()!=BMP_OK){
    // cout<<"here"<<endl;
    perror("ERROR: Failed to open BMP file.");
    return EXIT_FAILURE;
    }
 // creating BitMap object for the blurred image
    const unsigned int height = image.height();
    const unsigned int width = image.width();
    BitMap blurred(width, height);

    if (blurred.check_error()!=BMP_OK){
    // cout<<"or here"<<endl;
    perror("ERROR: Failed to open BMP file.");
    return EXIT_FAILURE;
    }

    // write the logic of thread, create an array of threads
    // array<pthread_t, thread_count> thread_ids{};
    vector<pthread_t> thread_ids(thread_count);

    // strong the arguments for each of the threads ina  vector
    vector<thread_args> thread_args_vec(thread_count);
    // I will divide the rows (height) of the image into (n-1) threads and put 
    // the remaining into the last thread
    int rows_per_thread = static_cast<int>(height)/thread_count;
    // int remaining_rows = height%thread_count;

    auto start_time = chrono::high_resolution_clock::now();
    // cout<<"start time: "<<start_time.time_since_epoch().count()<<endl;
    // creating the threads
    for (int i=0; i<thread_count;i++){
        // struct thread_args* args = new struct thread_args;
        // args->image = &image;
        // args->blurred = &blurred;
        // args->box_size = box_size;
        // args->start_row = i*rows_per_thread;
        // args->end_row = (i==thread_count-1)? height-1 : (i+1)*rows_per_thread - 1;

        thread_args_vec.at(i).image = &image;
        thread_args_vec.at(i).blurred = &blurred;
        thread_args_vec.at(i).box_size = box_size;
        thread_args_vec.at(i).start_row = i*rows_per_thread;
        thread_args_vec.at(i).end_row = (i==thread_count-1)
                                        ? static_cast<int>(height)-1 
                                        : (i+1)*rows_per_thread - 1;

        // thread spawning
        if (pthread_create(&thread_ids.at(i), nullptr, &threadmain, &thread_args_vec.at(i)) != 0) {
            cerr << "pthread_create failed" << endl;
        }
    }

    // waiting for all child threads to finish
    // (children may terminate out of order, but cleans up in order)
    for (int i = 0; i < thread_count; i++) {
        if (pthread_join(thread_ids.at(i), nullptr) != 0) {
            cerr << "pthread_join failed" << endl;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    cout << "Blurring completed in " << duration << " ms using " << thread_count << " threads.\n";

    // Output the blurred image to disk
    blurred.write_file(output_fname);
    if (blurred.check_error() != BMP_OK) {
        perror("ERROR: Failed to open BMP file.");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}