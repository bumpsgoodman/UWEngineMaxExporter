/*
* UWEngineMaxExporter 정의
*
* 작성자: bumpsgoodman
* 작성일: 2023.01.31
*/

#pragma once

struct ColorF
{
    float Red;
    float Green;
    float Blue;
    float Alpha;
};

struct UW3D
{
    char Magic[8]; // "uw3d"

    unsigned int NumVertices;
    unsigned int NumIndices;
    ColorF WireframeColor;
};