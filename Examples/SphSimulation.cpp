#include "SphSimulation.h"

#include "SphKernels.h"
#include <iostream>

namespace hlab {

using namespace std;

void SphSimulation::Update(float dt) {

    UpdateDensity();

    UpdateForces();

    for (int i = 0; i < m_particlesCpu.size(); i++) {

        if (m_particlesCpu[i].life < 0.0f)
            continue;

        m_particlesCpu[i].velocity += m_particlesCpu[i].force * dt / m_mass;
        m_particlesCpu[i].position += m_particlesCpu[i].velocity * dt;
    }
}

void SphSimulation::UpdateDensity() {

#pragma omp parallel for
    for (int i = 0; i < m_particlesCpu.size(); i++) {

        if (m_particlesCpu[i].life < 0.0f)
            continue;

        m_particlesCpu[i].density = 0.0f;

        // the summation over j includes all particles
        // i와 j가 같을 경우에도 고려한다는 의미
        // https://en.wikipedia.org/wiki/Smoothed-particle_hydrodynamics

        for (size_t j = 0; j < m_particlesCpu.size(); j++) {

            if (m_particlesCpu[j].life < 0.0f)
                continue;

            const float dist =
                (m_particlesCpu[i].position - m_particlesCpu[j].position)
                    .Length();

            if (dist >= m_radius)
                continue;

            m_particlesCpu[i].density +=
                m_mass * SphKernels::CubicSpline(dist * 2.0f / m_radius);
        }

        m_particlesCpu[i].pressure =
            m_pressureCoeff *
            (pow(m_particlesCpu[i].density / m_density0, 7.0f) - 1.0f);
    }
}

void SphSimulation::UpdateForces() {

#pragma omp parallel for
    for (int i = 0; i < m_particlesCpu.size(); i++) {

        if (m_particlesCpu[i].life < 0.0f)
            continue;

        Vector3 pressureForce(0.0f);
        Vector3 viscosityForce(0.0f);

        const float &rho_i = m_particlesCpu[i].density;
        const float &p_i = m_particlesCpu[i].pressure;
        const Vector3 &x_i = m_particlesCpu[i].position;
        const Vector3 &v_i = m_particlesCpu[i].velocity;

        for (size_t j = 0; j < m_particlesCpu.size(); j++) {

            if (m_particlesCpu[j].life < 0.0f)
                continue;

            if (i == j)
                continue;

            const float &rho_j = m_particlesCpu[j].density;
            const float &p_j = m_particlesCpu[j].pressure;
            const Vector3 &x_j = m_particlesCpu[j].position;
            const Vector3 x_ij = x_i - x_j;
            const Vector3 &v_j = m_particlesCpu[j].velocity;

            const float dist = (x_i - x_j).Length();

            if (dist >= m_radius)
                continue;

            if (dist < 1e-3f) // 수치 오류 방지
                continue;

            // 힌트: SphKernels::CubicSplineGrad() 사용
            const Vector3 gradPressure = Vector3(0.0f);

            const Vector3 laplacianVelocity = Vector3(0.0f);

            pressureForce -= m_mass / rho_i * gradPressure;
            viscosityForce += m_mass * m_viscosity * laplacianVelocity;
        }

        m_particlesCpu[i].force = pressureForce + viscosityForce;
    }
}

} // namespace hlab