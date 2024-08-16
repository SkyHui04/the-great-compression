
#include "include/image_compression/running_length_encoding.h"

const std::size_t RunningLengthEncoding::_PIXEL_BLOCK_SIZE = 3;

RunningLengthEncoding::RunningLengthEncoding(BaseLinearMapping* mapping, float threshold)
: BaseImageCompression(true), _mapping(mapping), _threshold(threshold)
{

}

RunningLengthEncoding::~RunningLengthEncoding()
{
    delete _mapping;
}

void RunningLengthEncoding::read(cv::Mat& image)
{
    int required_padding = 1;
    while (required_padding < std::max(image.cols, image.rows))
        required_padding <<= 1;
    setPaddedHeight(required_padding);
    setPaddedWidth(required_padding);
    for (int i = 0; i < MAX_NUM_CHANNELS; i++)
        _pixel_block_arrays[i].clear();
    BaseImageCompression::read(image);
}

void RunningLengthEncoding::write(cv::Mat& image, bool show_padding)
{
    BaseImageCompression::write(image, show_padding);
}

void RunningLengthEncoding::info() const
{
    BaseImageCompression::info();
}

void RunningLengthEncoding::readFrame(cv::Mat& frame, std::size_t frame_index)
{
    _mapping->preprocess(getPaddedHeight(), getPaddedWidth());
    float val;
    std::vector<_PixelBlock>& pixel_blocks = _pixel_block_arrays[frame_index];
    // Process first block
    pixel_blocks.push_back({1, frame.at<float>(_mapping->next())});
    float prev_val = pixel_blocks.back().value;
    assert(prev_val >= 0.);
    // Process the rest
    for (cv::Point pixel_loc = _mapping->next(); pixel_loc != POINT_END; pixel_loc = _mapping->next())
    {
        // MAKE SURE FIRST PIXEL IS NON-EMPTY
        val = frame.at<float>(pixel_loc);
        // std::cout << val << " " << pixel_loc << std::endl;
        if (val < 0.0f)
            val = prev_val;
        if (
            (std::fabs(val - prev_val) >= _threshold) || (pixel_blocks.back().frequency >= UINT16_MAX)
        )
        {
            // new block
            pixel_blocks.push_back({1, val});
            prev_val = val;
        }
        else
        {
            // add to block
            pixel_blocks.back().frequency++;
            // update arithm. mean (numerically stable)
            // https://dassencio.org/68
            pixel_blocks.back().value
                = pixel_blocks.back().value + (val - pixel_blocks.back().value) / pixel_blocks.back().frequency;
            prev_val = pixel_blocks.back().value;
        }
    }
    // update number of bytes
    setNumBytes(frame_index, _PIXEL_BLOCK_SIZE * pixel_blocks.size());
}

void RunningLengthEncoding::writeFrame(cv::Mat& frame, std::size_t frame_index)
{
    auto randomPixel = std::bind(std::uniform_real_distribution<float>(0.0f, 1.0f), std::default_random_engine());
    float randomized_color;
    _mapping->preprocess(getPaddedHeight(), getPaddedWidth());
    std::vector<_PixelBlock>& pixel_blocks = _pixel_block_arrays[frame_index];
    //pixel_loc != POINT_END
    cv::Point cur_pt;
    if (_random_colors)
    {
        for (_PixelBlock block : pixel_blocks)
        {
            randomized_color = randomPixel();
            for (int i = 0; i < block.frequency; i++)
            {
                cur_pt = _mapping->next();
                // std::cout << cur_pt << std::endl;
                if (cur_pt.y >= frame.rows || cur_pt.x >= frame.cols)
                    continue;
                frame.at<float>(cur_pt) = randomized_color;
                // std::cout << frame.at<float>(cur_pt) << std::endl;
            }
        }
    }
    else
    {
        for (_PixelBlock block : pixel_blocks)
        {
            for (int i = 0; i < block.frequency; i++)
            {
                cur_pt = _mapping->next();
                // std::cout << cur_pt << std::endl;
                if (cur_pt.y >= frame.rows || cur_pt.x >= frame.cols)
                    continue;
                frame.at<float>(cur_pt) = block.value;
                // std::cout << frame.at<float>(cur_pt) << std::endl;
            }
        }
    }
}

void RunningLengthEncoding::visualiseEncoding(cv::Mat& image, bool show_padding)
{
    _random_colors = true;
    RunningLengthEncoding::write(image, show_padding);
    _random_colors = false;
}

void RunningLengthEncoding::encode(std::ostream& file)
{
    BaseImageCompression::encode(file);
}

void RunningLengthEncoding::decode(std::istream& file)
{
    BaseImageCompression::decode(file);
}

void RunningLengthEncoding::encodeFrame(uchar* buffer, std::size_t frame_index, std::size_t begin, std::size_t end)
{
    std::size_t cur_pos = begin;
    // std::cout << (void*)buffer << std::endl;
    // std::cout << &locHWord(buffer, 0) << " " << &locHWord(buffer, begin) << " " << &locHWord(buffer, end) << std::endl;
    
    for (_PixelBlock block : _pixel_block_arrays[frame_index])
    {
        assert(cur_pos < end);
        //std::cout << cur_pos << " " << (uint16_t) block.frequency << " " << (uint16_t) (block.value * 255.) << std::endl;
        //std::cout << &locHWord(buffer, cur_pos) << std::endl;
        locHWord(buffer, cur_pos) = (uint16_t) block.frequency;
        locByte(buffer, cur_pos + 2) = (uchar) (block.value * 255.);  // TODO: check accuracy/ make it a general function
        //std::cout << cur_pos << " " << locHWord(buffer, cur_pos) << " " << (uint16_t) locByte(buffer, cur_pos + 2) << std::endl;
        cur_pos += _PIXEL_BLOCK_SIZE;
    }
    // assert(false);
}

void RunningLengthEncoding::decodeFrame(uchar* buffer, std::size_t frame_index, std::size_t begin, std::size_t end)
{
    _pixel_block_arrays[frame_index].clear();
    for (std::size_t cur_pos = begin; cur_pos < end; cur_pos += _PIXEL_BLOCK_SIZE)
    {
        _pixel_block_arrays[frame_index].push_back(
            {locHWord(buffer, cur_pos), (float) locByte(buffer, cur_pos + 2) / 255.0f}
        );
    }
}

