/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Andrew Philip

- All project requirements fully met? (YES or NO):
    Yes

- If no, please explain what you could not get to work:
    N/A

- Did you do any optional enhancements? If so, please explain:
    No
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <algorithm>
using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */ 
int get_int(fstream& stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)                     
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//


vector<vector<Pixel>> process_1(const vector<vector<Pixel>>& image)
{
    // Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
    int num_rows = image.size();
    int num_columns = image[0].size();
    
    // Define a new 2D vector the same size as the input 2D vector
    vector<vector<Pixel>> newvector;
    
    for (int i = 0; i < num_rows; i++)
    {
        vector<Pixel> row(num_columns);
        newvector.push_back(row);
    }
    
    // For each of the rows in the input 2D vector
    for (int i = 0; i < num_rows; i++)
    {
        // For each of the columns in the input 2D vector
        for (int j = 0; j < num_columns; j++)
        {
            // Get the color values for the pixel located at this row and column in the input 2D vector
            int redval = image[i][j].red;
            int greenval = image[i][j].green;
            int blueval = image[i][j].blue;
            // Perform the operation on the color values (refer to Runestone for this)
            double distance = sqrt(pow((j - num_columns/2.0),2.0) + pow((i - num_rows/2.0),2.0));
            double scaling_factor = (num_rows - distance)/num_rows;
            int newred = image[i][j].red * scaling_factor;
            int newgreen = image[i][j].green * scaling_factor;
            int newblue = image[i][j].blue * scaling_factor;

            // Save the new color values to the corresponding pixel located at this row and column in the new 2D vector
            newvector[i][j].red = newred;
            newvector[i][j].green = newgreen;
            newvector[i][j].blue = newblue;
        }
    }
      
    // Return the new 2D vector after the nested for loop is complete  
    return newvector;
}

vector<vector<Pixel>> process_2(const vector<vector<Pixel>>& image, double scaling_factor)
{
    // Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
    int num_rows = image.size();
    int num_columns = image[0].size();
    
    // Define a new 2D vector the same size as the input 2D vector
    vector<vector<Pixel>> newvector;
    
    for (int i = 0; i < num_rows; i++)
    {
        vector<Pixel> row(num_columns);
        newvector.push_back(row);
    }
    
    // For each of the rows in the input 2D vector
    for (int i = 0; i < num_rows; i++)
    {
        // For each of the columns in the input 2D vector
        for (int j = 0; j < num_columns; j++)
        {
            // Get the color values for the pixel located at this row and column in the input 2D vector
            int redval = image[i][j].red;
            int greenval = image[i][j].green;
            int blueval = image[i][j].blue;
            // Perform the operation on the color values (refer to Runestone for this)
            double average = (redval + greenval + blueval)/3;
            int newred;
            int newgreen;
            int newblue;
            
            if (average >= 170)
            {
                newred = int(255 - (255 - image[i][j].red)*scaling_factor);
                newgreen = int(255 - (255 - image[i][j].green)*scaling_factor);
                newblue = int(255 - (255 - image[i][j].blue)*scaling_factor);
            }
            else if (average <90)
            {
                newred = int(image[i][j].red*scaling_factor);
                newgreen = int(image[i][j].green*scaling_factor);
                newblue = int(image[i][j].blue*scaling_factor);
            }
            else
            {
                newred = image[i][j].red;
                newgreen = image[i][j].green;
                newblue = image[i][j].blue;
            }

            // Save the new color values to the corresponding pixel located at this row and column in the new 2D vector
            newvector[i][j].red = newred;
            newvector[i][j].green = newgreen;
            newvector[i][j].blue = newblue;
        }
    }
      
    // Return the new 2D vector after the nested for loop is complete  
    return newvector;
}

vector<vector<Pixel>> process_3(const vector<vector<Pixel>>& image)
{
    // Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
    int num_rows = image.size();
    int num_columns = image[0].size();
    
    // Define a new 2D vector the same size as the input 2D vector
    vector<vector<Pixel>> newvector;
    
    for (int i = 0; i < num_rows; i++)
    {
        vector<Pixel> row(num_columns);
        newvector.push_back(row);
    }
    
    // For each of the rows in the input 2D vector
    for (int i = 0; i < num_rows; i++)
    {
        // For each of the columns in the input 2D vector
        for (int j = 0; j < num_columns; j++)
        {
            // Get the color values for the pixel located at this row and column in the input 2D vector
            int redval = image[i][j].red;
            int greenval = image[i][j].green;
            int blueval = image[i][j].blue;
            // Perform the operation on the color values (refer to Runestone for this)
            int average = int((redval + greenval + blueval)/3);

            // Save the new color values to the corresponding pixel located at this row and column in the new 2D vector
            newvector[i][j].red = average;
            newvector[i][j].green = average;
            newvector[i][j].blue = average;
        }
    }
      
    // Return the new 2D vector after the nested for loop is complete  
    return newvector;
}

vector<vector<Pixel>> process_4(const vector<vector<Pixel>>& image)
{
    // Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
    int num_rows = image.size();
    int num_columns = image[0].size();
    int new_rows = num_columns;
    int new_columns = num_rows;
    
    // Define a new 2D vector, reversing the heigth and width
    vector<vector<Pixel>> newvector;
    
    for (int i = 0; i < new_rows; i++)
    {
        vector<Pixel> row(new_columns);
        newvector.push_back(row);
    }
    
    // For each of the rows in the input 2D vector
    int column_counter = new_columns-1;
    for (int i = 0; i < num_rows; i++)
    {
        // For each of the columns in the input 2D vector
        int row_counter = 0;
        for (int j = 0; j < num_columns; j++)
        {
            // Get the color values for the pixel located at this row and column in the input 2D vector
            int redval = image[i][j].red;
            int greenval = image[i][j].green;
            int blueval = image[i][j].blue;
            // Perform the operation on the color values (refer to Runestone for this)
            // Save the new color values to the corresponding pixel located at this row and column in the new 2D vector
            newvector[row_counter][column_counter].red = redval;
            newvector[row_counter][column_counter].green = greenval;
            newvector[row_counter][column_counter].blue = blueval;
            row_counter++;
        }
        column_counter--;
    }
      
    // Return the new 2D vector after the nested for loop is complete  
    return newvector;
}

vector<vector<Pixel>> process_5(const vector<vector<Pixel>>& image, int number)
{
    //calculate angle for conditionals
    int angle = int(number * 90);
    
    if (angle & 90 != 0)
    {
        cout << "angle must be a multiple of 90 degrees." <<endl;
        return image;
    }
    else if (angle % 360 == 0)
    {
        return image;
    }
    else if (angle % 360 == 90)
    {
        return process_4(image);
    }
    else if (angle % 360 == 180)
    {
        return process_4(process_4(image));
    }
    else
    {
        return process_4(process_4(process_4(image)));
    }
    
}

vector<vector<Pixel>> process_6(const vector<vector<Pixel>>& image, int xscale, int yscale)
{
    // Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
    int num_rows = image.size();
    int num_columns = image[0].size();
    int height = int(num_rows * yscale);
    int width = int(num_columns * xscale);
    
    // Define a new 2D vector with scaled height and width
    vector<vector<Pixel>> newvector;
    
    for (int i = 0; i < height; i++)
    {
        vector<Pixel> row(width);
        newvector.push_back(row);
    }
    
    // For each of the rows in the input 2D vector
    for (int i = 0; i < height; i++)
    {
        // For each of the columns in the input 2D vector
        for (int j = 0; j < width; j++)
        {
            // Get the color values for the pixel located at this row and column in the input 2D vector
            int redval = image[int(i/yscale)][int(j/xscale)].red;
            int greenval = image[int(i/yscale)][int(j/xscale)].green;
            int blueval = image[int(i/yscale)][int(j/xscale)].blue;
            // Perform the operation on the color values (refer to Runestone for this)
            // Save the new color values to the corresponding pixel located at this row and column in the new 2D vector
            newvector[i][j].red = redval;
            newvector[i][j].green = greenval;
            newvector[i][j].blue = blueval;
        }
    }
      
    // Return the new 2D vector after the nested for loop is complete  
    return newvector;
}

vector<vector<Pixel>> process_7(const vector<vector<Pixel>>& image)
{
    // Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
    int num_rows = image.size();
    int num_columns = image[0].size();
    
    // Define a new 2D vector the same size as the input 2D vector
    vector<vector<Pixel>> newvector;
    
    for (int i = 0; i < num_rows; i++)
    {
        vector<Pixel> row(num_columns);
        newvector.push_back(row);
    }
    
    // For each of the rows in the input 2D vector
    for (int i = 0; i < num_rows; i++)
    {
        // For each of the columns in the input 2D vector
        for (int j = 0; j < num_columns; j++)
        {
            // Get the color values for the pixel located at this row and column in the input 2D vector
            int redval = image[i][j].red;
            int greenval = image[i][j].green;
            int blueval = image[i][j].blue;
            // Perform the operation on the color values (refer to Runestone for this)
            double average = (redval + greenval + blueval)/3;
            int newred;
            int newgreen;
            int newblue;
            
            if (average >= 255/2)
            {
                newred = 255;
                newgreen = 255;
                newblue = 255;
            }
            else
            {
                newred = 0;
                newgreen = 0;
                newblue = 0;
            }

            // Save the new color values to the corresponding pixel located at this row and column in the new 2D vector
            newvector[i][j].red = newred;
            newvector[i][j].green = newgreen;
            newvector[i][j].blue = newblue;
        }
    }
      
    // Return the new 2D vector after the nested for loop is complete  
    return newvector;
}

vector<vector<Pixel>> process_8(const vector<vector<Pixel>>& image, double scaling_factor)
{
    // Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
    int num_rows = image.size();
    int num_columns = image[0].size();
    
    // Define a new 2D vector the same size as the input 2D vector
    vector<vector<Pixel>> newvector;
    
    for (int i = 0; i < num_rows; i++)
    {
        vector<Pixel> row(num_columns);
        newvector.push_back(row);
    }
    
    // For each of the rows in the input 2D vector
    for (int i = 0; i < num_rows; i++)
    {
        // For each of the columns in the input 2D vector
        for (int j = 0; j < num_columns; j++)
        {
            // Get the color values for the pixel located at this row and column in the input 2D vector
            int redval = image[i][j].red;
            int greenval = image[i][j].green;
            int blueval = image[i][j].blue;
            // Perform the operation on the color values (refer to Runestone for this)
            int newred = int(255 - (255 - redval)*scaling_factor);
            int newgreen = int(255 - (255 - greenval)*scaling_factor);
            int newblue = int(255 - (255 - blueval)*scaling_factor);

            // Save the new color values to the corresponding pixel located at this row and column in the new 2D vector
            newvector[i][j].red = newred;
            newvector[i][j].green = newgreen;
            newvector[i][j].blue = newblue;
        }
    }
      
    // Return the new 2D vector after the nested for loop is complete  
    return newvector;
}

vector<vector<Pixel>> process_9(const vector<vector<Pixel>>& image, double scaling_factor)
{
    // Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
    int num_rows = image.size();
    int num_columns = image[0].size();
    
    // Define a new 2D vector the same size as the input 2D vector
    vector<vector<Pixel>> newvector;
    
    for (int i = 0; i < num_rows; i++)
    {
        vector<Pixel> row(num_columns);
        newvector.push_back(row);
    }
    
    // For each of the rows in the input 2D vector
    for (int i = 0; i < num_rows; i++)
    {
        // For each of the columns in the input 2D vector
        for (int j = 0; j < num_columns; j++)
        {
            // Get the color values for the pixel located at this row and column in the input 2D vector
            int redval = image[i][j].red;
            int greenval = image[i][j].green;
            int blueval = image[i][j].blue;
            // Perform the operation on the color values (refer to Runestone for this)
            int newred = int(redval*scaling_factor);
            int newgreen = int(greenval*scaling_factor);
            int newblue = int(blueval*scaling_factor);

            // Save the new color values to the corresponding pixel located at this row and column in the new 2D vector
            newvector[i][j].red = newred;
            newvector[i][j].green = newgreen;
            newvector[i][j].blue = newblue;
        }
    }
      
    // Return the new 2D vector after the nested for loop is complete  
    return newvector;
}

vector<vector<Pixel>> process_10(const vector<vector<Pixel>>& image)
{
    // Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
    int num_rows = image.size();
    int num_columns = image[0].size();
    
    // Define a new 2D vector the same size as the input 2D vector
    vector<vector<Pixel>> newvector;
    
    for (int i = 0; i < num_rows; i++)
    {
        vector<Pixel> row(num_columns);
        newvector.push_back(row);
    }
    
    // For each of the rows in the input 2D vector
    for (int i = 0; i < num_rows; i++)
    {
        // For each of the columns in the input 2D vector
        for (int j = 0; j < num_columns; j++)
        {
            // Get the color values for the pixel located at this row and column in the input 2D vector
            int redval = image[i][j].red;
            int greenval = image[i][j].green;
            int blueval = image[i][j].blue;
            // Perform the operation on the color values (refer to Runestone for this)
            //find max value
            int max_value = 0;
            int arr[3] = {redval, greenval, blueval};
            for (int i = 0; i < 3; i++)
            {
                if (arr[i] >= max_value)
                {
                    max_value = arr[i];
                }
            }
            int newred;
            int newgreen;
            int newblue;
            
            if (redval + greenval + blueval >= 550)
            {
                newred = 255;
                newgreen = 255;
                newblue = 255;
            }
            
            if (redval + greenval + blueval <= 150)
            {
                newred = 0;
                newgreen = 0;
                newblue = 0;
            }
            
            else if (max_value == redval)
            {
                newred = 255;
                newgreen = 0;
                newblue = 0;
            }
            
            else if (max_value == greenval)
            {
                newred = 0;
                newgreen = 255;
                newblue = 0;
            }
            
            else 
            {
                newred = 0;
                newgreen = 0;
                newblue = 255;
            }

            // Save the new color values to the corresponding pixel located at this row and column in the new 2D vector
            newvector[i][j].red = newred;
            newvector[i][j].green = newgreen;
            newvector[i][j].blue = newblue;
        }
    }
      
    // Return the new 2D vector after the nested for loop is complete  
    return newvector;
}

int main()
{   
    bool done = false;
    string selection;
    string outputfilename;
    
    while (!done)
    {
        cout << "CSPB 1300 Image Processing Application" << endl;
        cout << "Enter input BMP filename: ";
        string filename;
        cin >> filename;

        cout << "IMAGE PROCESSING MENU" << endl;
        cout << "0) Change image (current: " << filename << ")" <<endl;;
        cout << "1) Vignette" << endl; 
        cout << "2) Clarendon" << endl; 
        cout << "3) Grayscale" << endl; 
        cout << "4) Rotate 90 degrees" << endl; 
        cout << "5) Rotate Multiple 90 degrees" << endl; 
        cout << "6) Enlarge" << endl; 
        cout << "7) High Contrast" << endl; 
        cout << "8) Lighten" << endl; 
        cout << "9) Darken" << endl; 
        cout << "10) Black, white, red, green, blue\n\n" << endl; 

        cout << "Enter menu selection (Q to quit): ";
        cin >> selection;

        // Read in BMP image file into a 2D vector (using read_image function)
        vector<vector<Pixel>> imageread = read_image(filename);

        // Call process function using the input 2D vector and save the result returned to a new 2D vector
        vector<vector<Pixel>> processed_image;
        
        if (selection == "0")
        {
            cout << "Change image selected." << endl;
            cout << "Enter new input BMP filename: ";
            cin >> filename;
            cout << "Successfully changed input image!" <<endl;
        }
        else if (selection == "1")
        {
            cout << "Vignette selected." << endl;
            cout << "Enter output file name: " << endl;
            string outputfile;
            cin >> outputfile;
            
            if (outputfile != filename)
            {
                processed_image = process_1(imageread);
                cout << "Vignette is successfully applied!" << endl;
                outputfilename = outputfile + ".bmp";
                bool imageresult = write_image(outputfilename, processed_image);
                if (!imageresult)
                {
                    cout << "Error: Process did not execute correctly." << endl;
                }
            }
            else
            {
                cout << "The output file cannot be the same as the input file. Please try again." << endl;
            }
        }
        else if (selection == "2")
        {
            cout << "Clarendon selected." << endl;
            cout << "Enter output file name: " << endl;
            string outputfile;
            cin >> outputfile;
            if (outputfile != filename)
            {
                cout << "Enter scaling factor between 0 and 1: " << endl;
                double scaling_factor;
                cin >> scaling_factor;
                processed_image = process_2(imageread, scaling_factor);
                cout << "Clarendon is successfully applied!" << endl;
                outputfilename = outputfile + ".bmp";
                bool imageresult = write_image(outputfilename, processed_image);
                if (!imageresult)
                {
                    cout << "Error: Process did not execute correctly." << endl;
                }
            }
            else
            {
                cout << "The output file cannot be the same as the input file. Please try again." << endl;
            }
        }
        else if (selection == "3")
        {
            cout << "Grayscale selected." << endl;
            cout << "Enter output file name: " << endl;
            string outputfile;
            cin >> outputfile;
            if (outputfile != filename)
            {
                processed_image = process_3(imageread);
                cout << "Grayscale is successfully applied!" << endl;
                outputfilename = outputfile + ".bmp";
                bool imageresult = write_image(outputfilename, processed_image);
                if (!imageresult)
                {
                    cout << "Error: Process did not execute correctly." << endl;
                }
            }
            else
            {
                cout << "The output file cannot be the same as the input file. Please try again." << endl;
            }
        }
        else if (selection == "4")
        {
            cout << "Rotate 90 degrees selected." << endl;
            cout << "Enter output file name: " << endl;
            string outputfile;
            cin >> outputfile;
            if (outputfile != filename)
            {
                processed_image = process_4(imageread);
                cout << "Rotate 90 degrees is successfully applied!" << endl;
                outputfilename = outputfile + ".bmp";
                bool imageresult = write_image(outputfilename, processed_image);
                if (!imageresult)
                {
                    cout << "Error: Process did not execute correctly." << endl;
                }
            }
            else
            {
                cout << "The output file cannot be the same as the input file. Please try again." << endl;
            }
        }
        else if (selection == "5")
        {
            cout << "Rotate by multiple 90 degrees selected." << endl;
            cout << "Enter output file name: " << endl;
            string outputfile;
            cin >> outputfile;
            if (outputfile != filename)
            {
                cout << "Enter number of 90 degree rotations: " << endl;
                int rotations;
                cin >> rotations;
                processed_image = process_5(imageread, rotations);
                cout << "Rotate by multiple 90 degrees is successfully applied!" << endl;
                outputfilename = outputfile + ".bmp";
                bool imageresult = write_image(outputfilename, processed_image);
                if (!imageresult)
                {
                    cout << "Error: Process did not execute correctly." << endl;
                }
            }
            else
            {
                cout << "The output file cannot be the same as the input file. Please try again." << endl;
            }
        }
        else if (selection == "6")
        {
            cout << "Enlarge selected." << endl;
            cout << "Enter output file name: " << endl;
            string outputfile;
            cin >> outputfile;
            if (outputfile != filename)
            {
                cout << "Enter X scale as an integer: " << endl;
                int xscale_input;
                cin >> xscale_input;
                cout << "Enter Y scale as an integer: " << endl;
                int yscale_input;
                cin >> yscale_input;
                processed_image = process_6(imageread, xscale_input, yscale_input);
                cout << "Successfully enlarged!" << endl;
                outputfilename = outputfile + ".bmp";
                bool imageresult = write_image(outputfilename, processed_image);
                if (!imageresult)
                {
                    cout << "Error: Process did not execute correctly." << endl;
                }
            }
            else
            {
                cout << "The output file cannot be the same as the input file. Please try again." << endl;
            }
        }
        else if (selection == "7")
        {
            cout << "High Contrast selected." << endl;
            cout << "Enter output file name: " << endl;
            string outputfile;
            cin >> outputfile;
            if (outputfile != filename)
            {
                processed_image = process_7(imageread);
                cout << "High Contrast is successfully applied!" << endl;
                outputfilename = outputfile + ".bmp";
                bool imageresult = write_image(outputfilename, processed_image);
                if (!imageresult)
                {
                    cout << "Error: Process did not execute correctly." << endl;
                }
            }
            else
            {
                cout << "The output file cannot be the same as the input file. Please try again." << endl;
            }
        }
        else if (selection == "8")
        {
            cout << "Lighten selected." << endl;
            cout << "Enter output file name: " << endl;
            string outputfile;
            cin >> outputfile;
            if (outputfile != filename)
            {
                cout << "Enter lightening scaling factor between 0 and 1: " << endl;
                double scaling_factor;
                cin >> scaling_factor;
                processed_image = process_8(imageread, scaling_factor);
                cout << "Lighten is successfully applied!" << endl;
                outputfilename = outputfile + ".bmp";
                bool imageresult = write_image(outputfilename, processed_image);
                if (!imageresult)
                {
                    cout << "Error: Process did not execute correctly." << endl;
                }
            }
            else
            {
                cout << "The output file cannot be the same as the input file. Please try again." << endl;
            }
        }
        else if (selection == "9")
        {
            cout << "Darken selected." << endl;
            cout << "Enter output file name: " << endl;
            string outputfile;
            cin >> outputfile;
            if (outputfile != filename)
            {
                cout << "Enter lightening scaling factor between 0 and 1: " << endl;
                double scaling_factor;
                cin >> scaling_factor;
                processed_image = process_9(imageread, scaling_factor);
                cout << "Darken is successfully applied!" << endl;
                outputfilename = outputfile + ".bmp";
                bool imageresult = write_image(outputfilename, processed_image);
                if (!imageresult)
                {
                    cout << "Error: Process did not execute correctly." << endl;
                }
            }
            else
            {
                cout << "The output file cannot be the same as the input file. Please try again." << endl;
            }
        }
        else if (selection == "10")
        {
            cout << "Black, white, red, green, blue has been selected." << endl;
            cout << "Enter output file name: " << endl;
            string outputfile;
            cin >> outputfile;
            if (outputfile != filename)
            {
                processed_image = process_10(imageread);
                cout << "Black, white, red, green blue is successfully applied!" << endl;
                outputfilename = outputfile + ".bmp";
                bool imageresult = write_image(outputfilename, processed_image);
                if (!imageresult)
                {
                    cout << "Error: Process did not execute correctly." << endl;
                }
            }
            else
            {
                cout << "The output file cannot be the same as the input file. Please try again." << endl;
            }
        }
        else if (selection == "Q")
        {
            cout << "Thank you for using my program." << endl;
            cout << "Quitting..." << endl;
            done = true;
            break;
        }
        else
        {
            cout << "You have entered an invalid option. Please try again." << endl;
        }
        
     }
    
    return 0;
    

}