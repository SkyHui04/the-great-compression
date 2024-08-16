
#include <iostream>
#include <opencv2/opencv.hpp>
#include <queue>
#include "include/linear_mapping/base_linear_mapping.h"

class MortonCurve : public BaseLinearMapping
{
public:
    MortonCurve();
    virtual ~MortonCurve() = default;
    virtual void preprocess(std::size_t height, std::size_t width);
    virtual cv::Point next();
private:
    // O(n) space O(n) time implementation
    void generatePointsHelper(int yi, int yj, int xi, int xj);
    std::queue<cv::Point> points;
};
