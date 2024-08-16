
#include <iostream>
#include <opencv2/opencv.hpp>
#include <queue>
#include "include/linear_mapping/base_linear_mapping.h"

class HilbertCurve : public BaseLinearMapping
{
public:
    HilbertCurve();
    virtual ~HilbertCurve() = default;
    virtual void preprocess(std::size_t height, std::size_t width);
    virtual cv::Point next();
private:
    // O(n) space O(n) time implementation
    // TODO: rewrite using L-system to reduce runtime and space usage
    void generatePointsHelper(int yi, int yj, int xi, int xj, int mode);
    std::queue<cv::Point> points;
};



