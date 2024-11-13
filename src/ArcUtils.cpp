#include "ArcUtils.h"

#include "DxfModel.h"
#include <cmath>

namespace {

    static const auto PI = std::atan(1.) * 4;

    double inRange_0_2PI(double theta)
    {
        theta = std::fmod(theta, 2 * PI);
        if (theta < 0)
            theta += 2 * PI;
        return theta;
    }
}

bool isNull(double theta, double epsilon)
{ //
    return std::fabs(theta) <= epsilon;
}

bool isNull2PI(double theta, double epsilon)
{
    theta = inRange_0_2PI(theta);
    if (std::fabs(theta) <= epsilon)
        return true;
    if (std::fabs(theta - 2 * PI) <= epsilon)
        return true;
    return false;
}

void normalize(double& theta1, double& theta2, bool direct)
{
    if (isNull2PI(theta1 - theta2)) {
        const double deltaTheta = isNull(theta1 - theta2) ? 0 : 2 * PI;
        theta1                  = inRange_0_2PI(theta1);
        theta2                  = theta1 + deltaTheta;
    }
    else {
        theta1 = inRange_0_2PI(theta1);
        theta2 = inRange_0_2PI(theta2);
        if (!direct)
            std::swap(theta1, theta2);
        if (theta2 < theta1)
            theta2 += 2 * PI;
    }
}