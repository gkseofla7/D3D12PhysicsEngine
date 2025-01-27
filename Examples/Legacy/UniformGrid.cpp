#include "UniformGrid.h"

namespace hlab {

void UniformGrid::IterateAll(
    const std::function<void(ivec3 ijk, size_t idx)> &func) {
    for (int k = 0; k < m_res[2]; k++)
        for (int j = 0; j < m_res[1]; j++)
            for (int i = 0; i < m_res[0]; i++) {
                ivec3 ijk(i, j, k);
                func(ivec3(i, j, k), Index(ijk));
            }
}

void UniformGrid::IterateAllPar(
    const std::function<void(ivec3 ijk, size_t idx)> &func) {
    for (int k = 0; k < m_res[2]; k++)
        for (int j = 0; j < m_res[1]; j++)
#pragma omp parallel for
            for (int i = 0; i < m_res[0]; i++) {
                ivec3 ijk(i, j, k);
                func(ivec3(i, j, k), Index(ijk));
            }
}

void UniformGrid::Iterate(
    const std::function<void(ivec3 ijk, size_t idx)> &func) {
    for (int k = m_pad; k < m_res[2] - m_pad; k++)
        for (int j = m_pad; j < m_res[1] - m_pad; j++)
            for (int i = m_pad; i < m_res[0] - m_pad; i++) {
                ivec3 ijk(i, j, k);
                func(ivec3(i, j, k), Index(ijk));
            }
}

void UniformGrid::IteratePar(
    const std::function<void(ivec3 ijk, size_t idx)> &func) {
    for (int k = m_pad; k < m_res[2] - m_pad; k++)
        for (int j = m_pad; j < m_res[1] - m_pad; j++)
#pragma omp parallel for
            for (int i = m_pad; i < m_res[0] - m_pad; i++) {
                ivec3 ijk(i, j, k);
                func(ivec3(i, j, k), Index(ijk));
            }
}

} // namespace hlab