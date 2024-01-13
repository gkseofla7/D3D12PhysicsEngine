#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>

namespace hlab {

using DirectX::SimpleMath::Vector3;
using std::vector;

class SphSimulation {
  public:
    struct Particle {
        Vector3 position = Vector3(0.0f);
        Vector3 velocity = Vector3(0.0f);
        Vector3 force = Vector3(0.0f);
        float density = 0.0f;
        float pressure = 0.0f;

        Vector3 color = Vector3(1.0f);
        float life = 0.0f;
        float size = 1.0f;
    };

    void Update(float dt);
    void UpdateDensity();
    void UpdateForces();

    vector<Particle> m_particlesCpu;
    float m_radius = 1.0f / 16.0f;
    float m_mass = 1.0f;
    float m_pressureCoeff = 1.0f;
    float m_density0 = 1.0f;
    float m_viscosity = 0.1f;

  private:
};
} // namespace hlab