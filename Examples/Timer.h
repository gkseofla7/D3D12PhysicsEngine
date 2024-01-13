#pragma once

#include <chrono>
#include <d3d11.h>
#include <iostream>

namespace hlab {
class Timer {
  public:
    Timer() {}
    Timer(ComPtr<ID3D11Device> &device) {
        D3D11_QUERY_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Query = D3D11_QUERY_TIMESTAMP;
        desc.MiscFlags = 0;
        ThrowIfFailed(device->CreateQuery(&desc, &m_startQuery));
        ThrowIfFailed(device->CreateQuery(&desc, &m_stopQuery));
        desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        ThrowIfFailed(device->CreateQuery(&desc, &m_disjointQuery));
    }

    void Start(ComPtr<ID3D11DeviceContext> &context, bool measureGPU) {

        m_measureGPU = measureGPU;

        double m_elapsedTimeGPU = 0.0;
        double m_elapsedTimeCPU = 0.0f;

        m_startTimeCPU = std::chrono::high_resolution_clock::now();

        if (m_measureGPU) {
            context->Begin(m_disjointQuery.Get());
            context->End(m_startQuery.Get());
        }
    }

    void End(ComPtr<ID3D11DeviceContext> &context) {

        if (m_measureGPU) {

            // GPU Profiling in DX11 with Queries
            // https://therealmjp.github.io/posts/profiling-in-dx11-with-queries/

            context->End(m_stopQuery.Get());
            context->End(m_disjointQuery.Get());

            while (context->GetData(m_disjointQuery.Get(), NULL, 0, 0) ==
                   S_FALSE)
                continue;
            D3D11_QUERY_DATA_TIMESTAMP_DISJOINT tsDisjoint;
            context->GetData(m_disjointQuery.Get(), &tsDisjoint,
                             sizeof(tsDisjoint), 0);

            // The timestamp returned by ID3D11DeviceContext::GetData for a
            // timestamp query is only reliable if Disjoint is FALSE.
            // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_query_data_timestamp_disjoint

            if (!tsDisjoint.Disjoint) {
                UINT64 startTime, stopTime;
                context->GetData(m_startQuery.Get(), &startTime, sizeof(UINT64),
                                 0);
                context->GetData(m_stopQuery.Get(), &stopTime, sizeof(UINT64),
                                 0);

                m_elapsedTimeGPU = (double(stopTime - startTime) /
                                    double(tsDisjoint.Frequency)) *
                                   1000.0;
            } else {
                m_elapsedTimeGPU = -1.0;
            }
        }

        m_elapsedTimeCPU =
            (std::chrono::high_resolution_clock::now() - m_startTimeCPU)
                .count() /
            double(1e6); // microsec -> millisec

        if (m_measureGPU) {
            std::cout << "GPU: " << m_elapsedTimeGPU << ", ";
        }

        std::cout << "CPU: " << m_elapsedTimeCPU << std::endl;
    }

  public:
    double m_elapsedTimeCPU = 0.0f;
    double m_elapsedTimeGPU = 0.0;

  protected:
    ComPtr<ID3D11Query> m_startQuery, m_stopQuery, m_disjointQuery;

    decltype(std::chrono::high_resolution_clock::now()) m_startTimeCPU;

    bool m_measureGPU = false;
};
} // namespace hlab