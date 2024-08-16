
#include "include/linear_mapping/hilbert_curve.h"

HilbertCurve::HilbertCurve()
: BaseLinearMapping()
{

}

void HilbertCurve::generatePointsHelper(int yi, int yj, int xi, int xj, int mode)
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
    // mode: 0 (U), 1 (C), 2 (n), 3 (])
    switch (mode)
    {
    case 0:
        generatePointsHelper(yi, mid_y, xi, mid_x, 3);
        generatePointsHelper(mid_y, yj, xi, mid_x, 0);
        generatePointsHelper(mid_y, yj, mid_x, xj, 0);
        generatePointsHelper(yi, mid_y, mid_x, xj, 1);
        break;
    case 1:
        generatePointsHelper(mid_y, yj, mid_x, xj, 2);
        generatePointsHelper(mid_y, yj, xi, mid_x, 1);
        generatePointsHelper(yi, mid_y, xi, mid_x, 1);
        generatePointsHelper(yi, mid_y, mid_x, xj, 0);
        break;
    case 2:
        generatePointsHelper(mid_y, yj, mid_x, xj, 1);
        generatePointsHelper(yi, mid_y, mid_x, xj, 2);
        generatePointsHelper(yi, mid_y, xi, mid_x, 2);
        generatePointsHelper(mid_y, yj, xi, mid_x, 3);
        break;
    case 3:
        generatePointsHelper(yi, mid_y, xi, mid_x, 0);
        generatePointsHelper(yi, mid_y, mid_x, xj, 3);
        generatePointsHelper(mid_y, yj, mid_x, xj, 3);
        generatePointsHelper(mid_y, yj, xi, mid_x, 2);
        break;
    }
}

cv::Point HilbertCurve::next()
{
    if (points.empty())
        return POINT_END;
    cv::Point result = points.front();
    points.pop();
    return result;
}

void HilbertCurve::preprocess(std::size_t height, std::size_t width)
{
    points = {};
    generatePointsHelper(0, height, 0, width, 0);
}
