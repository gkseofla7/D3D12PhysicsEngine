#pragma once

#define GLM_FORCE_INTRINSICS // GLM_FORCE_PURE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include <glm/glm.hpp>
#include <glm/simd/common.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>

namespace hlab {

using glm::ivec3; // i32vec3
using glm::vec3;
using std::vector;

class UniformGrid {
  public:
    void Initialize(ivec3 resolution, float widthWorld) {

        m_res = resolution;
        m_numSlice = m_res.x * m_res.y;
        m_numCells = size_t(m_res.x * m_res.y * m_res.z);
        m_dx = widthWorld / m_res[0];
        m_invDx = m_res[0] / widthWorld; // 1.0f / m_dx;

        // 격자 공간 범위 [0,width]x[0,height]x[0,depth]
        // 격자의 (0, 0, 0)은 World의 (0, 0, 0)은 동일
        // Scale은 있으나 이동, 회전은 없음 (추가 가능)
    }

    vec3 PosWorldToGrid(vec3 posWorld) {
        // 스케일, 회전은 없다고 가정
        return posWorld * m_invDx;
    }

    template <typename T> T ScaleWorldToGrid(T length) {
        return length * m_invDx;
    }

    // Padding 포함 모든 Cell Iterate
    void IterateAll(const std::function<void(ivec3 ijk, size_t idx)> &func);
    void IterateAllPar(const std::function<void(ivec3 ijk, size_t idx)> &func);

    // Padding 미포함
    void Iterate(const std::function<void(ivec3 ijk, size_t idx)> &func);
    void IteratePar(const std::function<void(ivec3 ijk, size_t idx)> &func);

    vec3 CellCenter(const ivec3 &ijk) { return vec3(ijk) + 0.5f; }

    bool IsPad(const ivec3 &ijk) {
        for (int d = 0; d < 3; d++) {
            if (ijk[d] < m_pad)
                return true;
            if (ijk[d] >= m_res[d] - m_pad - 1)
                return true;
        }
        return false;
    }

    size_t Index(const ivec3 &ijk) {
        return size_t(ijk[0] + m_res[0] * ijk[1] + m_numSlice * ijk[2]);
    }

    size_t IndexClamp(ivec3 ijk) {
        ijk = clamp(ijk, {0, 0, 0}, m_res - 1);
        return size_t(ijk[0] + m_res[0] * ijk[1] + m_numSlice * ijk[2]);
    }

    template <class TT> TT Lerp(const vec3 &pos, const vector<TT> &arr);
    template <class TT>
    void LerpClamp(const vec3 &pos, const vector<TT> &arr, TT &value);

    ivec3 m_res = ivec3(0); // width, height, depth
    float m_dx = 0.0f;      // dx, dy, dz 동일하다고 가정
    float m_invDx = 1.0f;

    int m_pad = 1; // CFL에 따라서 조절

    size_t m_numSlice = 0;
    size_t m_numCells = 0;

  private:
};

template <class TT>
TT UniformGrid::Lerp(const vec3 &pos, const vector<TT> &arr) {

    using namespace glm;

    const vec3 minCell = floor(pos - 0.5f);
    const size_t ix = IndexClamp(minCell);
    const vec3 posCell = pos - (minCell + 0.5f);
    const float &x = posCell.x;
    const float &y = posCell.y;
    const float &z = posCell.z;
    const float w000 = (1.0f - x) * (1.0f - y) * (1.0f - z);
    const float w100 = x * (1.0f - y) * (1.0f - z);
    const float w010 = (1.0f - x) * y * (1.0f - z);
    const float w001 = (1.0f - x) * (1.0f - y) * z;
    const float w101 = x * (1.0f - y) * z;
    const float w011 = (1.0f - x) * y * z;
    const float w110 = x * y * (1.0f - z);
    const float w111 = x * y * z;

    const TT &v000 = arr[ix];
    const TT &v100 = arr[ix + 1];
    const TT &v010 = arr[ix + m_res[0]];
    const TT &v001 = arr[ix + m_numSlice];
    const TT &v101 = arr[ix + 1 + m_numSlice];
    const TT &v011 = arr[ix + m_res[0] + m_numSlice];
    const TT &v110 = arr[ix + 1 + m_res[0]];
    const TT &v111 = arr[ix + 1 + m_res[0] + m_numSlice];

    return v000 * w000 + v100 * w100 + v010 * w010 + v001 * w001 + v101 * w101 +
           v011 * w011 + v110 * w110 + v111 * w111;
}

template <class TT>
void UniformGrid::LerpClamp(const vec3 &pos, const vector<TT> &arr, TT &value) {

    using namespace glm;

    const vec3 minCell = floor(pos - 0.5f);
    const size_t ix = IndexClamp(minCell);
    const vec3 posCell = pos - (minCell + 0.5f);

    const TT &v000 = arr[ix];
    const TT &v100 = arr[ix + 1];
    const TT &v010 = arr[ix + m_res[0]];
    const TT &v001 = arr[ix + m_numSlice];
    const TT &v101 = arr[ix + 1 + m_numSlice];
    const TT &v011 = arr[ix + m_res[0] + m_numSlice];
    const TT &v110 = arr[ix + 1 + m_res[0]];
    const TT &v111 = arr[ix + 1 + m_res[0] + m_numSlice];

    TT maxValue = v000;
    TT minValue = v000;
    maxValue = glm::max(v100, maxValue);
    maxValue = glm::max(v010, maxValue);
    maxValue = glm::max(v001, maxValue);
    maxValue = glm::max(v101, maxValue);
    maxValue = glm::max(v011, maxValue);
    maxValue = glm::max(v110, maxValue);
    maxValue = glm::max(v111, maxValue);

    minValue = glm::min(v100, minValue);
    minValue = glm::min(v010, minValue);
    minValue = glm::min(v001, minValue);
    minValue = glm::min(v101, minValue);
    minValue = glm::min(v011, minValue);
    minValue = glm::min(v110, minValue);
    minValue = glm::min(v111, minValue);

    value = glm::clamp(value, minValue, maxValue);
}

} // namespace hlab