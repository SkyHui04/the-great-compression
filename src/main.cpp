
#include <iostream>
#include <opencv2/opencv.hpp>
#include "include/image_compression/running_length_encoding.h"
#include "include/linear_mapping/hilbert_curve.h"
#include "include/linear_mapping/morton_curve.h"

using namespace cv;
using namespace std;

string extractFilename(const string& filename);

BaseImageCompression* chooseAlgorithm();
bool compress(const string& filename);
bool decompress(const string& filename);

int main(int argc, char** argv)
{
    int mode = 0;
    std::string filename;
    while (true)
    {
        cout << "Select mode..." << endl
            << "\t0: compression" << endl
            << "\t1: decompression" << endl
            << "\t2: exit" << endl;
        cout << "mode: ";
        cin >> mode;
        switch (mode)
        {
        case 0:
            // compression
            cout << "filename: ";
            cin >> filename;
            compress(filename);
            break;
        case 1:
            // decompression
            cout << "filename: ";
            cin >> filename;
            decompress(filename);
            break;
        case 2:
            return 0;
        
        default:
            cout << "Invalid mode." << endl;
            break;
        }
    }
    return 0;
}

string extractFilename(const string& filename)
{
    return filename.substr(0, filename.find('.'));
}

BaseImageCompression* chooseAlgorithm()
{
    int algorithm;
    int linear_mapping;
    double threshold;
    BaseImageCompression* encoder;

    cout << "Choose an algorithm..." << endl
        << "\t0: running-length encoding" << endl
        << "algorithm: ";
    cin >> algorithm;

    switch (algorithm)
    {
    case 0:
        // RLE
        cout << "Choose a linear mapping method..." << endl
            << "\t0: Hilbert curve" << endl
            << "\t1: Morton curve" << endl
            << "linear mapping: ";
        cin >> linear_mapping;
        cout << "(threshold determines the 'lossiness' of compression; value < 0.0039 leads to loseless compression)\n";
        cout << "Pixel value threshold (within [0, 1]) for lossy compression: ";
        cin >> threshold;

        switch (linear_mapping)
        {
        case 0:
            // Hilbert curve
            encoder = new RunningLengthEncoding(new HilbertCurve, threshold);
            break;
        
        case 1:
            // Morton Curve
            encoder = new RunningLengthEncoding(new MortonCurve, threshold);
            break;
        
        default:
            cout << "Invalid linear mapping." << endl;
            return nullptr;
            break;
        }
        break;
    
    default:
        cout << "Invalid algorithm." << endl;
        return nullptr;
    }
    return encoder;
}

bool compress(const string& filename)
{
    string read_path = "data/input/" + filename;
    string write_path = "data/compressed/" + extractFilename(filename) + ".compressed";
    Mat input_img = imread(read_path, IMREAD_COLOR);
    if (input_img.empty())
    {
        cout << "Cannot open image. Please check file location and file format." << endl;
        return false;
    }

    BaseImageCompression* encoder = chooseAlgorithm();
    if (encoder == nullptr)
        return false;

    encoder->read(input_img);
    encoder->info();

    ofstream encoded_file(write_path);
    encoder->encode(encoded_file);

    delete encoder;
    return true;
}

bool decompress(const string& filename)
{
    string read_path = "data/compressed/" + filename;
    string write_path = "data/output/" + extractFilename(filename) + ".png";
    string write_path_visualise = "data/visualise/" + extractFilename(filename) + ".png";

    ifstream encoded_file(
        read_path,
        std::ios::in | std::ios::binary
    );

    if (!encoded_file.is_open())
    {
        cout << "Cannot open file. Please check file location." << endl;
        return false;
    }

    bool show_padding;
    cout << "Show padding in the result? (yes = 1, no = 0): ";
    cin >> show_padding;

    BaseImageCompression* decoder = chooseAlgorithm();
    if (decoder == nullptr)
        return false;

    decoder->decode(encoded_file);
    encoded_file.close();
    decoder->info();

    Mat output_img;
    decoder->write(output_img, show_padding);
    cv::imwrite(write_path, output_img);
    if (typeid(*decoder) == typeid(RunningLengthEncoding))
        dynamic_cast<RunningLengthEncoding*>(decoder)->visualiseEncoding(output_img, show_padding);
    cv::imwrite(write_path_visualise, output_img);

    delete decoder;
    return true;
}
