
#include "include/image_compression/base_compression.h"

const std::size_t BaseImageCompression::_METADATA_SIZE = 128;

BaseImageCompression::BaseImageCompression(bool padding)
: _padding(padding)
{

}

void BaseImageCompression::read(cv::Mat& image)
{
    // accept an image with CV_8U or CV_8UC? for now
    std::cout << " -------------------- Read image begins -------------------- \n";
    Timer timer;
    timer.begin();
    _num_channels = image.channels();
    _height = image.rows;
    _width = image.cols;

    cv::Mat frames[MAX_NUM_CHANNELS];
    if (getNumChannels() > 1)
    {
        cv::split(image, frames);
    }
    else
    {
        frames[0] = image.clone();
    }

    for (int i = 0; i < getNumChannels(); i++)
    {
        // internally use CV_32F
        frames[i].convertTo(frames[i], CV_32F, 1./255., 0.);
        if (_padding)
            addPadding(frames[i], frames[i], _padded_height, _padded_width);
        readFrame(frames[i], i);
    }
    timer.end();
    timer.report();
    std::cout << " -------------------- Read image ends -------------------- \n";
}

void BaseImageCompression::write(cv::Mat& image, bool show_padding)
{
    std::cout << " -------------------- Write image begins -------------------- \n";
    Timer timer;
    timer.begin();
    cv::Mat frames[MAX_NUM_CHANNELS];
    if (show_padding)
    {
        for (int i = 0; i < getNumChannels(); i++)
        {
            frames[i] = cv::Mat::zeros(cv::Size{(int)getPaddedWidth(), (int)getPaddedHeight()}, CV_32F);
            writeFrame(frames[i], i);
            frames[i].convertTo(frames[i], CV_8U, 255., 0.);
        }
    }
    else
    {
        for (int i = 0; i < getNumChannels(); i++)
        {
            frames[i] = cv::Mat::zeros(cv::Size{(int)_width, (int)_height}, CV_32F);
            writeFrame(frames[i], i);
            frames[i].convertTo(frames[i], CV_8U, 255., 0.);
        }
    }
    
    if (getNumChannels() > 1)
    {
        cv::merge(frames, getNumChannels(), image);
    }
    else
    {
        image = frames[0];
    }
    timer.end();
    timer.report();
    std::cout << " -------------------- Write image ends -------------------- \n";
}

void BaseImageCompression::info() const
{
    std::cout << " -------------------- Info begins -------------------- \n";
    std::size_t total_resolution = getHeight() * getWidth() * getNumChannels();
    std::cout << "\tOriginal resolution: "
        << getHeight() << " x " << getWidth() << " x " << getNumChannels()
        << " = " << total_resolution << "\n";
    if (_padding)
    {
        std::cout << "\tPadded resolution: "
            << getPaddedHeight() << " x " << getPaddedWidth() << " x " << getNumChannels()
            << " = " << getPaddedHeight() * getPaddedWidth() * getNumChannels() << "\n";
    }
    std::size_t total_bytes = _METADATA_SIZE;
    std::cout << "\tNumber of bytes used: \n";
    std::cout << "\t\tmetadata: " << _METADATA_SIZE << "\n";
    for (int i = 0; i < getNumChannels(); i++)
    {
        std::cout << "\t\tframe " << i << ": " << getNumBytes(i) << "\n";
        total_bytes += getNumBytes(i);
    }
    std::cout << "\tTotal: " << total_bytes << "\n";
    std::cout << "\tCompression ratio: " << (long double) total_resolution / total_bytes << "\n";
    std::cout << " -------------------- Info ends -------------------- \n";
}

void BaseImageCompression::encode(std::ostream& file)
{
    assert(loaded());
    std::cout << " -------------------- Encode image begins -------------------- \n";
    Timer timer;
    timer.begin();
    //
    // Write metadata (preserve 128B, 8B to store each var):
    //  (unsigned long) num_channels
    //  (unsigned long) height
    //  (unsigned long) width
    //  (bool) padding
    //  (unsigned long) padded_height
    //  (unsigned long) padded_width
    //  (unsigned long []) num_bytes (@per frame) [warning: variable length]
    //
    std::memset(compression_buffer, 0, _METADATA_SIZE);
    locDWord(compression_buffer, (0 << 3)) = getNumChannels();
    // std::cout << (void*) &compression_buffer << " " << (void*) &locDWord(compression_buffer, (0 << 3)) << std::endl;
    locDWord(compression_buffer, (1 << 3)) = getHeight();
    locDWord(compression_buffer, (2 << 3)) = getWidth();
    locDWord(compression_buffer, (3 << 3)) = (std::size_t) _padding;
    locDWord(compression_buffer, (4 << 3)) = getPaddedHeight();
    locDWord(compression_buffer, (5 << 3)) = getPaddedWidth();
    std::size_t total_bytes = _METADATA_SIZE;
    for (std::size_t i = 0; i < getNumChannels(); i++)
    {
        // std::cout << "setting bytes " << i << " " << getNumBytes(i) << "\n";
        locDWord(compression_buffer, ((i + 6) << 3)) = getNumBytes(i);
        total_bytes += getNumBytes(i);
    }
    assert(total_bytes <= BUFFER_SIZE);
    std::size_t write_pos = _METADATA_SIZE;
    for (std::size_t i = 0; i < getNumChannels(); i++)
    {
        // std::cout << (void*) &compression_buffer << std::endl;
        encodeFrame(&compression_buffer[0], i, write_pos, write_pos + getNumBytes(i));
        write_pos += getNumBytes(i);
    }
    file.write((char*) compression_buffer, write_pos);
    timer.end();
    timer.report();
    std::cout << " -------------------- Encode image ends -------------------- \n";
}

void BaseImageCompression::decode(std::istream& file)
{
    std::cout << " -------------------- Decode image begins -------------------- \n";
    Timer timer;
    timer.begin();
    file.read((char*) compression_buffer, BUFFER_SIZE);
    //
    // Read metadata
    //  (unsigned long) num_channels
    //  (unsigned long) height
    //  (unsigned long) width
    //  (bool) padding
    //  (unsigned long) padded_height
    //  (unsigned long) padded_width
    //  (unsigned long []) num_bytes (@per frame) [warning: variable length]
    //
    _num_channels = locDWord(compression_buffer, (0 << 3));
    _height = locDWord(compression_buffer, (1 << 3));
    _width = locDWord(compression_buffer, (2 << 3));
    _padding = locDWord(compression_buffer, (3 << 3));
    _padded_height = locDWord(compression_buffer, (4 << 3));
    _padded_width = locDWord(compression_buffer, (5 << 3));
    std::size_t total_bytes = _METADATA_SIZE;
    for (std::size_t i = 0; i < getNumChannels(); i++)
    {
        // std::cout << "setting bytes " << i << " " << locDWord(compression_buffer, i+6) << "\n";
        setNumBytes(i, locDWord(compression_buffer, ((i + 6) << 3)));
        total_bytes += getNumBytes(i);
    }
    assert(total_bytes <= BUFFER_SIZE);
    std::size_t write_pos = _METADATA_SIZE;
    for (std::size_t i = 0; i < getNumChannels(); i++)
    {
        decodeFrame(compression_buffer, i, write_pos, write_pos + getNumBytes(i));
        write_pos += getNumBytes(i);
    }
    timer.end();
    timer.report();
    std::cout << " -------------------- Decode image ends -------------------- \n";
}

void addPadding(cv::Mat& input_img, cv::Mat& output_img, std::size_t padded_height, std::size_t padded_width)
{
    // std::cout << padded_height << " " << input_img.rows << " " << padded_width << " " << input_img.cols << std::endl;
    assert((padded_height >= input_img.rows) && (padded_width >= input_img.cols));
    cv::copyMakeBorder(
        input_img, output_img,
        0,
        padded_height - input_img.rows,
        0,
        padded_width - input_img.cols,
        CV_HAL_BORDER_CONSTANT,
        cv::Scalar(-1.)
    );
}
