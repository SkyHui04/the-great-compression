
#include "include/linear_mapping/morton_curve.h"

MortonCurve::MortonCurve()
: BaseLinearMapping()
{

}

void MortonCurve::generatePointsHelper(int yi, int yj, int xi, int xj)
{
    //cout << xi << " " << xj << " " << yi << " " << yj << endl;
    int pixels = (yj - yi) * (xj - xi);
    if (pixels == 0)
        return;
    if (pixels == 1)
    {
        points.push(cv::Point(xi, yi));
        return;
    }
    int mid_y, mid_x;
    mid_y = yi + (yj - yi) / 2;
    mid_x = xi + (xj - xi) / 2;
    generatePointsHelper(yi, mid_y, xi, mid_x);
    generatePointsHelper(yi, mid_y, mid_x, xj);
    generatePointsHelper(mid_y, yj, xi, mid_x);
    generatePointsHelper(mid_y, yj, mid_x, xj);
}

cv::Point MortonCurve::next()
{
    if (points.empty())
        return POINT_END;
    cv::Point result = points.front();
    points.pop();
    return result;
}

void MortonCurve::preprocess(std::size_t height, std::size_t width)
{
    points = {};
    generatePointsHelper(0, height, 0, width);
}
