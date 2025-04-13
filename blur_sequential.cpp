#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <chrono>

using namespace std;

#include "qdbmp.hpp"

int main(int argc, char* argv[]) {
 
  int box_size = stoi(argv[3]);
  if (argc != 4){
    // int box_size = stoi(argv[3]);
    cerr << "Usage: " << argv[0] << " <input file> <output file> <box size>" << endl;
    return EXIT_FAILURE;
  }

  if (box_size<=0){
    cerr << "ERROR: Box size must be positive." << endl;
    return EXIT_FAILURE;
  }

  string input_fname{argv[1]};
  string output_fname{argv[2]};
  // Construct a BitMap object using the input file specified
  BitMap image(input_fname);

  if (image.check_error()!=BMP_OK){
    perror("ERROR: Failed to open BMP file.");
    return EXIT_FAILURE;
  }

  // creating Bitmap object for the blurred image
  const unsigned int height = image.height();
  const unsigned int width = image.width();

  BitMap blurred(width, height);

  if (blurred.check_error()!=BMP_OK){
    perror("ERROR: Failed to open BMP file.");
    return EXIT_FAILURE;
  }
  // debugging
  // printf("width: %d, height: %d\n", width, height);
  // cout<<"box size is : "<<box_size<<endl;

  auto start_time = chrono::high_resolution_clock::now();

  // Loop through each pixel and turn into blurred
  for (size_t y = 0; y<height; y++){
    for (size_t x = 0; x<width; x++){
      // get the pixel
      // RGB color = image.get_pixel(x, y);
      int red_sum = 0;
      int green_sum = 0;
      int blue_sum = 0;
      int count = 0;
      // determining the neighbours as we have to loop through eah of poxels 

      for (int i = -box_size; i <= box_size; i++){
        for (int j = -box_size; j<=box_size; j++){
          // bounds check is imp here
          size_t nei_x = x + i;
          size_t nei_y = y+j;

          if (nei_x  >= 0 && nei_x < width && nei_y >= 0 && nei_y < height){
            RGB neighbour = image.get_pixel(nei_x, nei_y);
            // if (color==neighbour){
            //   cout<<"this is the same pixel"<<endl;
            // }
            red_sum += neighbour.red;
            green_sum += neighbour.green;
            blue_sum += neighbour.blue;
            count++;
          }
        }
      }
      // calculating the average
      RGB blurred_pixels {
        static_cast<UCHAR>(red_sum/count),
        static_cast<UCHAR>(green_sum/count),
        static_cast<UCHAR>(blue_sum/count)
      };

      blurred.set_pixel(x, y, blurred_pixels);
      
    }
  }

  auto end_time = chrono::high_resolution_clock::now();
  auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
  cout << "Time taken to blur the image sequentially without threading: " << duration.count() << " milliseconds" << endl;
  // writing to the disk after eveything is done form the updating task
  blurred.write_file(output_fname);
  if (blurred.check_error()!=BMP_OK){
    perror("ERROR: Failed to open BMP file.");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}