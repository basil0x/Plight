#pragma once

#include <utility>
#include <vector>


std::vector<std::pair<Vec2, Vec2>> XSDL_LightCalculate
                     (std::vector<struct Polygon> &polygons,
                      std::vector<struct Light> &lights);

void XSDL_LightUpdate();

void XSDL_LightRender();


