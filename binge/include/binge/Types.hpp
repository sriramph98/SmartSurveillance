#pragma once

#include <ctime>
#include <vector>
#include <string>
#include <opencv2/core/mat.hpp>

struct FrameSize {
    int width;
    int height;
};

using FrameData = cv::Mat;

struct Frame {
    double timestamp;
    FrameData* frameData;
    FrameSize frameSize;
    std::vector<std::string> objects;
};