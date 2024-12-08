#pragma once

#include "pbrt.h"

namespace pbrt
{
    void pbrtInit(const Options& opt);
    void pbrtCleanup();

    void pbrtWorldBegin();
    void pbrtWorldEnd();

    void pbrtParseFile(std::string filename);
}