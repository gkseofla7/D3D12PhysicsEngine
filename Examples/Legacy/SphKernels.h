#pragma once

#include <assert.h>
#include <cmath>

namespace hlab {

using std::pow;

class SphKernels {
  public:
    // Eq(5) in "SPH Fluids in Computer Graphics"  by Ihmsen et al.
    // 주의: 유효한 최대 q가 1이 아니라 2입니다.
    static float CubicSpline(const float q) {

        assert(q >= 0.0f);

        constexpr float coeff = 3.0f / (2.0f * 3.141592f);

        if (q < 1.0f)
            return coeff * (2.0f / 3.0f - q * q + 0.5f * q * q * q);
        else if (q < 2.0f)
            return coeff * pow(2.0f - q, 3.0f) / 6.0f;
        else // q >= 2.0f
            return 0.0f;
    }

    /*
    * 	static Vector3r gradW(const Vector3r &r)
        {
            Vector3r res;
            const Real rl = r.norm();
            const Real q = rl / m_radius;
            if ((rl > 1.0e-9) && (q <= 1.0))
            {
                Vector3r gradq = r / rl;
                gradq /= m_radius;
                if (q <= 0.5)
                {
                    res = m_l*q*((Real) 3.0*q - static_cast<Real>(2.0))*gradq;
                }
                else
                {
                    const Real factor = static_cast<Real>(1.0) - q;
                    res = m_l*(-factor*factor)*gradq;
                }
            }
            else
                res.setZero();

            return res;
        }
    */

    static float CubicSplineGrad(const float q) {

        assert(q >= 0.0f);

        constexpr float coeff = 3.0f / (2.0f * 3.141592f);

        if (q < 1.0f)
            return coeff * (-2.0f * q + 1.5f * q * q);
        else if (q < 2.0f)
            return coeff * -0.5f * (2.0f - q) * (2.0f - q);
        else // q >= 2.0f
            return 0.0f;
    }
}; // class SphKernels

} // namespace hlab
