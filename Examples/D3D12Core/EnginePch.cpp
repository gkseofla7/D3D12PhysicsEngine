#include "EnginePch.h"
#include "Engine.h"
#include "GameCore/DaerimGame.h"
namespace dengine {
	unique_ptr<Engine> GEngine = std::make_unique<DaerimGame>();
	std::thread::id g_MainThreadId;
}

