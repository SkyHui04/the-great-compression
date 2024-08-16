
#ifndef BASE_COMPRESSION
#define BASE_COMPRESSION
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "include/general_helpers.h"

const std::size_t BUFFER_SIZE = 64 << 20;
static uchar compression_buffer[BUFFER_SIZE];  // 64MB compression buffer
const std::size_t MAX_NUM_CHANNELS = 4UL;

#define locByte(arr, i)     *(uint8_t*)  (&arr[i])
#define locHWord(arr, i)    *(uint16_t*) (&arr[i])
#define locWord(arr, i)     *(uint32_t*) (&arr[i])
#define locDWord(arr, i)    *(uint64_t*) (&arr[i])


/*
 * Base class for image compressors
 * 
 * Usage:
 *  - (1): load an image into the compressor and encode it into a compressed binary file
 *  - (2): usage (1) but reverse
 * 
 * Incoming images are splited into different color channels for processing.
 * To implement a compression algorithm, the following functions must be overridden:
 *  - readFrame
 *  - writeFrame
 *  - encodeFrame
 *  - decodeFrame
 * 
 * The read/write functions process single-channel float32 image frames.
 * The encode/decode functions process the binary data on the buffer, stored as a uchar array.
 * 
 */
class BaseImageCompression
{
public:
    BaseImageCompression(bool padding);
    virtual ~BaseImageCompression() {}

    // load image into compressor, does compression meanwhile
    virtual void read(cv::Mat& image);

    // overwrite a Mat object with the loaded image
    virtual void write(cv::Mat& image, bool show_padding);

    // encode the compressed image into binary file
    virtual void encode(std::ostream& file);

    // decode binary file and load image into compressor
    virtual void decode(std::istream& file);

    // get data dimensions and compression summary (e.g. compression ratio)
    virtual void info() const;

    bool loaded() const
        { return (bool) _num_channels; }
    std::size_t getNumChannels() const
        { return _num_channels; }
    std::size_t getHeight() const
        { return _height; }
    std::size_t getWidth() const
        { return _width; }
    std::size_t getPaddedHeight() const
        { return _padded_height; }
    std::size_t getPaddedWidth() const
        { return _padded_width; }
    void setPaddedHeight(std::size_t val)
        { _padded_height = val; }
    void setPaddedWidth(std::size_t val)
        { _padded_width = val; }
    std::size_t getNumBytes(std::size_t frame_index) const
        { return _num_bytes[frame_index]; }
    void setNumBytes(std::size_t frame_index, std::size_t num_bytes)
        { _num_bytes[frame_index] = num_bytes; }
    
private:
    // load frame into compressor
    // Note: frame is a single-channel float32 image
    // Note: the number of bytes required for the compressed frame must be updated using setNumBytes
    virtual void readFrame(cv::Mat& frame, std::size_t frame_index) = 0;

    // save compressed image into frame
    // Note: frame MUST be a single-channel float32 image
    virtual void writeFrame(cv::Mat& frame, std::size_t frame_index) = 0;

    // write buffer from begin (inclusive) to end (exclusive) with compressed image data
    virtual void encodeFrame(uchar* buffer, std::size_t frame_index, std::size_t begin, std::size_t end) = 0;

    // read buffer from begin (inclusive) to end (exclusive) and load image data into compressor
    virtual void decodeFrame(uchar* buffer, std::size_t frame_index, std::size_t begin, std::size_t end) = 0;
    
    static const std::size_t _METADATA_SIZE;
    std::size_t _num_bytes[MAX_NUM_CHANNELS];
    bool _padding;
    std::size_t _num_channels = 0;
    std::size_t _height = 0;
    std::size_t _width = 0;
    std::size_t _padded_height = 0;
    std::size_t _padded_width = 0;
};

void addPadding(cv::Mat& input_img, cv::Mat& output_img, std::size_t padded_height, std::size_t padded_width);

#endif // BASE_COMPRESSION
