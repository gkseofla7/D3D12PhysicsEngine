#pragma once
#include <map>
#include <string>
#include "GeometryGenerator.h"
#include "D3D11Utils.h"
#include "GraphicsCommon.h"

namespace hlab {
using namespace std;
class AnimStatus
{
public:
	static void AddAnimPath(int InActorId, string InPathName);
	static void AddAnimStateToAnim(int InActorId, int InState, string InAnimName);
	static void PlayAnimation(string InAnimName);
	static void UpdateAnimation(ComPtr<ID3D11DeviceContext>& context, Actor* InActor, int InState,
		int frame, int type = 0);
private:
	static map<int ,map<int, string>> m_animStateToAnim;
	static map<int, string> m_pathMap;
	static map<int, AnimationData> m_aniData;

};

}