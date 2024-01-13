#pragma once

/*
 * 선형 시스템 예시를 위해 최소한의 기능만 구현된 CPU 버전입니다.
 * 다음 강의의 GPU 버전과 1:1 대응되지 않습니다.
 */

#include <Eigen/IterativeLinearSolvers>
#include <execution>
#include <iostream>

#include "UniformGrid.h"

namespace hlab {

class FluidSimulationCPU {
  public:
    void Initialize(int width, int height, int depth) {

        m_grid.Initialize({width, height, depth}, 10.0f);

        m_density.resize(m_grid.m_numCells);
        m_velocity.resize(m_grid.m_numCells);
        m_temperature.resize(m_grid.m_numCells);
        m_pressure.resize(m_grid.m_numCells);
        m_temp.resize(m_grid.m_numCells);
        m_tempVec.resize(m_grid.m_numCells);
        m_temp2.resize(m_grid.m_numCells);
        m_tempVec2.resize(m_grid.m_numCells);
        m_cellType.resize(m_grid.m_numCells);

        m_cfl = 0.5f;

        // Spherical source
        m_sourcePosWorld = vec3(0.3f, 5.0f, 5.0f);
        m_sourceRadiusWorld = 1.0f;
        m_sourceVelocityWorld = vec3(4.0f, 0.0f, 0.0f);
    }

    void Update(float dt) {

        dt = glm::min(0.1f, dt); // 디버깅시 너무 느려지는 것 방지

        using namespace std;

        const int numSubsteps = 2;
        for (int i = 0; i < numSubsteps; i++) {

            const float dtSub = dt / numSubsteps;
            Sourcing(dtSub);
            Projection();
            Advection(dtSub);
        }

        // CFL condition 적용 예시
        // float dtStep = 0.0f;
        // int count = 0;
        // while (dtStep < dt) {
        //    const float dtSub =
        //        glm::min(dt - dtStep, 1.0f / m_maxSpeed * m_cfl);
        //    Sourcing(dtSub);
        //    Projection();
        //    Advection(dtSub);
        //    dtStep += dtSub;
        //    count++;
        //}
        // std::cout << "Count = " << count << ", MaxSpeed = " << m_maxSpeed
        //          << std::endl;
    }

    void VorticityConfinement(float dt) {

        // Compute vorticity
        auto &vorticity = m_tempVec;
        m_grid.IteratePar([&](ivec3 ijk, size_t idx) {
            // Vorticity
            // curl (u, v, w) = (dw/dy - dv/dz, du/dz - dw/dx, dv/dx - du/dy)
            const size_t adj[6] = {idx + 1,                  // plus x
                                   idx - 1,                  // minus x
                                   idx + m_grid.m_res[0],    // plus y
                                   idx - m_grid.m_res[0],    // minus y
                                   idx + m_grid.m_numSlice,  // plus z
                                   idx - m_grid.m_numSlice}; // minus z

            const float dwdy = m_velocity[adj[2]].z - m_velocity[adj[3]].z;
            const float dvdz = m_velocity[adj[4]].y - m_velocity[adj[5]].y;
            const float dudz = m_velocity[adj[4]].x - m_velocity[adj[5]].x;
            const float dwdx = m_velocity[adj[0]].z - m_velocity[adj[1]].z;
            const float dvdx = m_velocity[adj[0]].y - m_velocity[adj[1]].y;
            const float dudy = m_velocity[adj[2]].x - m_velocity[adj[3]].x;

            vorticity[idx] = vec3(dwdy - dvdz, dudz - dwdx, dvdx - dudy);
        });

        // Apply vorticity
        m_grid.IteratePar([&](ivec3 ijk, size_t idx) {
            const size_t adj[6] = {idx + 1,                  // plus x
                                   idx - 1,                  // minus x
                                   idx + m_grid.m_res[0],    // plus y
                                   idx - m_grid.m_res[0],    // minus y
                                   idx + m_grid.m_numSlice,  // plus z
                                   idx - m_grid.m_numSlice}; // minus z

            vec3 eta =
                vec3(length(vorticity[adj[0]]) - length(m_velocity[adj[1]]),
                     length(vorticity[adj[2]]) - length(m_velocity[adj[3]]),
                     length(vorticity[adj[4]]) - length(m_velocity[adj[5]]));

            float l = glm::length(eta);

            if (l > 1e-3) {

                vec3 N = eta / l;

                m_velocity[idx] +=
                    1.2f * glm::cross(N, vorticity[idx]) * dt * m_density[idx];
            }
        });
    }

    void Sourcing(float dt) {

        using namespace glm;

        VorticityConfinement(dt);

        const vec3 buoyancy = vec3(0.0f, 2.0f, 0.0f);
        const float diss = 0.01f; // density dissipation

        // Source sphere
        vec3 sourcePosGrid = m_grid.PosWorldToGrid(m_sourcePosWorld);
        float sourceRadiusGrid = m_grid.ScaleWorldToGrid(m_sourceRadiusWorld);
        vec3 sourceVelocity = m_grid.ScaleWorldToGrid(m_sourceVelocityWorld);

        m_cellCount = 0;
        m_grid.IterateAll([&](ivec3 ijk, size_t idx) {
            m_density[idx] = glm::max(0.0f, m_density[idx] - diss * dt);
            m_velocity[idx] += m_density[idx] * buoyancy * dt;

            // Object
            const float sourceDist =
                length(vec3(sourcePosGrid - m_grid.CellCenter(ijk)));
            if (sourceDist < sourceRadiusGrid) { // 구형 Source
                m_cellType[idx] = -2;            // Neumann
                m_density[idx] = 1.0f;
                m_velocity[idx] = sourceVelocity;
            } else if (ijk[1] < m_grid.m_pad) { // 바닥
                m_cellType[idx] = -2;           // Neumann boundary condition
            } else if (m_grid.IsPad(ijk)) {     // 그 외 가장자리
                m_cellType[idx] = -1;           // Dirichlet boundary condtion
            } else {
                m_cellType[idx] = m_cellCount;
                m_cellCount += 1;
            }
        });
    }

    void Diffuse() {} // Ignore viscosity

    void TestLinearSolver() {
        // Eigen::ConjugateGradient
        // https://eigen.tuxfamily.org/dox/classEigen_1_1ConjugateGradient.html
        // Sparse Matrix
        // http://www.eigen.tuxfamily.org/dox/group__TutorialSparse.html

        using namespace Eigen;

        int n = 3;
        VectorXf x(n), b(n);
        SparseMatrix<float> A(n, n);
        A.reserve(VectorXi::Constant(n, 2));
        A.insert(0, 0) = 1.0f;
        A.insert(1, 1) = 2.0f;
        A.insert(2, 2) = 3.0f;
        // A.makeCompressed(); // Optional

        b(0) = 2.8f;
        b(1) = 4.2f;
        b(2) = 6.7f;

        // fill A and b
        ConjugateGradient<SparseMatrix<float>, Lower | Upper> cg;
        cg.setMaxIterations(20);
        cg.setTolerance(1e-5f);
        cg.compute(A);
        x = cg.solve(b);

        std::cout << "Iterations:     " << cg.iterations() << std::endl;
        std::cout << "Estimated error: " << cg.error() << std::endl;
        std::cout << x << std::endl;

        exit(0);
    }

    void Projection() {
        // TestLinearSolver();

        /*
         * 1. Make Ax = b
         * 2. Solve for x (여기서 x는 pressure)
         * 3. Update velocity
         */

        using namespace Eigen;

        Eigen::initParallel();

        VectorXf x(m_cellCount), b(m_cellCount);
        SparseMatrix<float> A(m_cellCount, m_cellCount);
        A.reserve(VectorXi::Constant(m_cellCount, 7));

        // Compute divergence and make A matrix
        m_grid.Iterate([&](ivec3 ijk, size_t idx) {
            const int &cellId = m_cellType[idx];

            if (cellId >= 0) { // Skip non-full cells

                const size_t ijkAdj[6] = {idx + 1,
                                          idx - 1,
                                          idx + m_grid.m_res[0],
                                          idx - m_grid.m_res[0],
                                          idx + m_grid.m_numSlice,
                                          idx - m_grid.m_numSlice};

                float &div = b(cellId);
                float &diag = A.insert(cellId, cellId);

                div = 0.0f;
                diag = 0.0f;

                for (int d = 0; d < 6; d++) {

                    const size_t &adj = ijkAdj[d];

                    if (m_cellType[adj] == -1) { // Neumann
                        if (d % 2 == 0) {
                            div += 2.0f * m_velocity[adj][d / 2] -
                                   m_velocity[idx][d / 2];
                        } else {
                            div -= 2.0f * m_velocity[adj][d / 2] -
                                   m_velocity[idx][d / 2];
                        }
                    } else {
                        if (d % 2 == 0) {
                            div += m_velocity[adj][d / 2];
                        } else {
                            div -= m_velocity[adj][d / 2];
                        }
                    }

                    if (m_cellType[adj] != -2) { // Not Neumann
                        diag -= 1.0f;
                        if (m_cellType[adj] != -1) { // Not Dirichlet
                            A.insert(cellId, m_cellType[adj]) = 1.0f;
                        }
                    }
                }

            } // End of Skip non-full cells
        });

        // for (int i = 0; i < b.size(); i++) {
        //     if (b(i) != 0.0f)
        //         std::cout << b(i) << " ";
        // }
        // std::cout << std::endl;

        A.makeCompressed(); // Optional

        ConjugateGradient<SparseMatrix<float>, Lower | Upper> cg;
        cg.setMaxIterations(20);
        cg.setTolerance(1e-5f);
        cg.compute(A);
        x = cg.solve(b);

        std::cout << "Iterations:     " << cg.iterations() << std::endl;
        // std::cout << "Estimated error: " << cg.error() << std::endl;
        // for (int i = 0; i < x.size(); i++) {
        //     if (x(i) != 0.0f)
        //         std::cout << x(i) << " ";
        // }
        // std::cout << std::endl;
        // exit(0);

        m_maxSpeed = 0.0f;

        // Update velocity from pressure gradient
        m_grid.IteratePar([&](ivec3 ijk, size_t idx) {
            const int &cellId = m_cellType[idx];

            if (cellId >= 0) { // Skip non-full cells

                const size_t ijkAdj[6] = {idx + 1,
                                          idx - 1,
                                          idx + m_grid.m_res[0],
                                          idx - m_grid.m_res[0],
                                          idx + m_grid.m_numSlice,
                                          idx - m_grid.m_numSlice};

                vec3 gradP(0.0f);

                for (int d = 0; d < 6; d++) {

                    const size_t &adj = ijkAdj[d];

                    if (m_cellType[adj] == -2) { // Neumann
                        if (d % 2 == 0) {
                            gradP[d / 2] += x(cellId);
                        } else {
                            gradP[d / 2] -= x(cellId);
                        }
                    } else if (m_cellType[adj] >= 0) {
                        if (d % 2 == 0) {
                            gradP[d / 2] += x(m_cellType[adj]);
                        } else {
                            gradP[d / 2] -= x(m_cellType[adj]);
                        }
                    }
                }

                m_velocity[idx] -= gradP * 0.5f;

                // Remove leaking through Neumann boundary
                for (int d = 0; d < 6; d++) {
                    const size_t &adj = ijkAdj[d];
                    if (m_cellType[adj] == -2) { // Neumann
                        m_velocity[idx][d / 2] = m_velocity[adj][d / 2];
                        m_velocity[adj][d / 2] = 2.0f * m_velocity[adj][d / 2] -
                                                 m_velocity[idx][d / 2];
                    } else if (m_cellType[adj] == -1) { // Dirichlet
                        m_velocity[adj][d / 2] = m_velocity[idx][d / 2];
                    }
                }

                m_maxSpeed = glm::max(
                    glm::max(glm::max(m_maxSpeed, abs(m_velocity[idx][0])),
                             abs(m_velocity[idx][1])),
                    abs(m_velocity[idx][2]));

            } // End of Skip non-full cells
        });
    }

    void Advection(float dt) {
        using namespace std;

        copy(execution::par, m_density.begin(), m_density.end(),
             m_temp.begin());
        copy(execution::par, m_velocity.begin(), m_velocity.end(),
             m_tempVec.begin());

        const auto &densityTemp = m_temp;
        const auto &velocityTemp = m_tempVec;
        auto &densityTemp2 = m_temp2;
        auto &velocityTemp2 = m_tempVec2;

        // Forward
        m_grid.IteratePar([&](ivec3 ijk, size_t idx) {
            if (m_cellType[idx] >= 0) {

                const vec3 backPos =
                    m_grid.CellCenter(ijk) - velocityTemp[idx] * dt;

                m_density[idx] = m_grid.Lerp(backPos, densityTemp) * 0.999f;
                m_velocity[idx] = m_grid.Lerp(backPos, velocityTemp) * 0.999f;
                // TODO: Lerp multiple arrays to reuse w000~w111
            }
        });

        // Backward
        /*
        m_grid.IteratePar([&](ivec3 ijk, size_t idx) {
            if (m_cellType[idx] >= 0) {
                const vec3 forwardPos =
                    m_grid.CellCenter(ijk) + velocityTemp[idx] * dt;

                densityTemp2[idx] = m_grid.Lerp(forwardPos, m_density);
                //velocityTemp2[idx] = m_grid.Lerp(forwardPos, m_velocity);
            }
        });

        // Compensate
        m_grid.IteratePar([&](ivec3 ijk, size_t idx) {
            if (m_cellType[idx] >= 0) {
                const vec3 backPos =
                    m_grid.CellCenter(ijk) - velocityTemp[idx] * dt;

                m_density[idx] += 0.5f * (densityTemp[idx] - densityTemp2[idx]);
                //m_velocity[idx] +=
                  //  0.5f * (velocityTemp[idx] - velocityTemp2[idx]);

                // Clamp
                m_grid.LerpClamp(backPos, densityTemp, m_density[idx]);
                //m_grid.LerpClamp(backPos, velocityTemp, m_velocity[idx]);
            }
        });
        */
    }

    UniformGrid m_grid;
    vector<float> m_density;
    vector<float> m_pressure;

  private:
    vector<vec3> m_velocity; // GridSpace Scale
    vector<float> m_temperature;
    vector<float> m_temp, m_temp2;
    vector<vec3> m_tempVec, m_tempVec2;
    vector<int> m_cellType; // -1: Dirichlet, -2: Neumann, 0+: Full cell
    float m_maxSpeed = 0.0f;
    float m_cfl = 0.5f;
    int m_cellCount = 0;

    // 구형 Source
    float m_sourceRadiusWorld = 0.0f;
    vec3 m_sourcePosWorld = vec3(0.0f);
    vec3 m_sourceVelocityWorld = vec3(0.0f);
};

} // namespace hlab