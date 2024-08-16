
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <random>
#include "base_compression.h"
#include "include/linear_mapping/base_linear_mapping.h"

/* 
 * Map image onto a linear array and perform running-length encoding on it
 */
class RunningLengthEncoding : public BaseImageCompression
{
public:
    // mapping: a dynamically allocated BaseLinearMapping object, do NOT use pointer to static object
    RunningLengthEncoding(BaseLinearMapping* mapping, float threshold);
    virtual ~RunningLengthEncoding();

    virtual void read(cv::Mat& image);
    virtual void write(cv::Mat& image, bool show_padding = false);
    virtual void visualiseEncoding(cv::Mat& image, bool show_padding = false);
    virtual void encode(std::ostream& file);
    virtual void decode(std::istream& file);
    virtual void info() const;

private:
    virtual void readFrame(cv::Mat& frame, std::size_t frame_index);
    virtual void writeFrame(cv::Mat& frame, std::size_t frame_index);
    virtual void encodeFrame(uchar* buffer, std::size_t frame_index, std::size_t begin, std::size_t end);
    virtual void decodeFrame(uchar* buffer, std::size_t frame_index, std::size_t begin, std::size_t end);
    BaseLinearMapping* _mapping;
    float _threshold;
    bool _random_colors = false;
    struct _PixelBlock
    {
        std::size_t frequency;
        float value;
    };
    std::vector<_PixelBlock> _pixel_block_arrays[MAX_NUM_CHANNELS];

    static const std::size_t _PIXEL_BLOCK_SIZE;
};
