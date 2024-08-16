
#ifndef BASE_LINEAR_MAPPING
#define BASE_LINEAR_MAPPING
#include <iostream>
#include <opencv2/opencv.hpp>

const cv::Point POINT_END = {-1, -1};

// TODO: improve interface because usage is weird
class BaseLinearMapping
{
public:
    BaseLinearMapping() = default;
    virtual ~BaseLinearMapping() = default;
    virtual void preprocess(std::size_t height, std::size_t width) = 0;
    virtual cv::Point next() = 0;
    
private:
};

#endif // BASE_LINEAR_MAPPING
