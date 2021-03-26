#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
//#include <glm/gtx/string_cast.hpp>

#include <limits>
#include <memory>

// Types aliasing
using uint = unsigned int;
using uchar = unsigned char;
using byte = uchar;
using lint = long long;


using color3 = glm::dvec3;
using color4 = glm::dvec4;
using icolor3 = glm::ivec3;
using icolor4 = glm::ivec4;
using color = color3;
using icolor = icolor3;

using vec3 = glm::dvec3;
using vec4 = glm::dvec4;

using mat3 = glm::dmat3;
using mat4 = glm::dmat4;

using point3 = glm::dvec3;
using point4 = glm::dvec4;


using barycentric = vec3;


using std::shared_ptr;
using std::make_shared;

