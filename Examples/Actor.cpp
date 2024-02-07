#include "Actor.h"
#include "Camera.h"
#include "ModelLoadHelper.h"
namespace hlab {
    Actor::Actor() {}
    Actor::Actor(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
         const string& basePath, const string & filename)
    {
        Initialize(device, context, basePath, filename);
    }
    void Actor::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
         const string&basePath, const string & filename)
    {
        InitBoundingKey();
        m_isInitialized = ModelLoadHelper::LoadModelData(device, context, basePath, filename, m_model);
        m_basePath = basePath;
        m_filename = filename;
    }
    bool Actor::MsgProc(WPARAM wParam, shared_ptr<Actor> InActivateActore)
    {
        if (m_keyBinding.find(wParam) != m_keyBinding.end())
        {
            m_keyBinding[wParam]();
            return true;
        }
        return false;
    }
    //Camera 관련
	void Actor::ActiveCaemera()
	{ 
        m_camera->UpdatePosDir(m_cameraCorrection* m_actorConsts.GetCpu().world);
	}
    void Actor::UpdateCemeraCorrection(Vector3 deltaPos)
    {
        m_cameraCorrection = Matrix::CreateTranslation(deltaPos);
    }

    //Actor 위치 관련
	void Actor::UpdateWorldRow(const Matrix& worldRow)
	{
        this->m_worldRow = worldRow;
        this->m_worldITRow = worldRow;
        m_worldITRow.Translation(Vector3(0.0f));
        m_worldITRow = m_worldITRow.Invert().Transpose();

        // 바운딩스피어 위치 업데이트
        // 스케일까지 고려하고 싶다면 x, y, z 스케일 중 가장 큰 값으로 스케일
        // 구(sphere)라서 회전은 고려할 필요 없음
        m_boundingSphere.Center = this->m_worldRow.Translation();

        m_actorConsts.GetCpu().world = worldRow.Transpose();
        m_actorConsts.GetCpu().worldIT = m_worldITRow.Transpose();
        m_actorConsts.GetCpu().worldInv = m_actorConsts.GetCpu().world.Invert();
	}

    void Actor::UpdateConstantBuffers(ComPtr<ID3D11Device>& device,
        ComPtr<ID3D11DeviceContext>& context) {
        if (m_isVisible) {
            m_model->UpdateConstantBuffers(device, context);
            m_actorConsts.Upload(context);
        }
    }
    void Actor::Render(ComPtr<ID3D11DeviceContext>& context)
    {
        if (m_isInitialized == false)
        {
            m_isInitialized = ModelLoadHelper::GetModelData( m_basePath, m_filename, m_model);
        }
        if (m_model != nullptr && m_isInitialized == true)
        {
            m_model->Render(context);
        }

    }


}