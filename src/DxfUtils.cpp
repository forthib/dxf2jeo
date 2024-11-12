#include "DxfUtils.h"

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

    bool isNull(double theta, double epsilon = std::numeric_limits<double>::epsilon())
    { //
        return std::fabs(theta) <= epsilon;
    }

    bool isNull2PI(double theta, double epsilon = std::numeric_limits<double>::epsilon())
    {
        theta = inRange_0_2PI(theta);
        if (std::fabs(theta) <= epsilon)
            return true;
        if (std::fabs(theta - 2 * PI) <= epsilon)
            return true;
        return false;
    }
}

DxfArc normalize(DxfArc arc, bool direct)
{
    if (isNull2PI(arc.theta1 - arc.theta2)) {
        const double deltaTheta = isNull(arc.theta1 - arc.theta2) ? 0 : 2 * PI;
        arc.theta1              = inRange_0_2PI(arc.theta1);
        arc.theta2              = arc.theta1 + deltaTheta;
    }
    else {
        arc.theta1 = inRange_0_2PI(arc.theta1);
        arc.theta2 = inRange_0_2PI(arc.theta2);

        double deltaTheta = arc.theta2 - arc.theta1;
        if (deltaTheta > 0 && !direct)
            deltaTheta -= 2 * PI;
        else if (deltaTheta < 0 && direct)
            deltaTheta += 2 * PI;

        arc.theta2 = arc.theta1 + deltaTheta;
    }

    return arc;
}