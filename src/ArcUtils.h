#pragma once

#include <limits>

bool isNull(double theta, double epsilon = std::numeric_limits<double>::epsilon());
bool isNull2PI(double theta, double epsilon = std::numeric_limits<double>::epsilon());
void normalize(double& theta1, double& theta2, bool direct);