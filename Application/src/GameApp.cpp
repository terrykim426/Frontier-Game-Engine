#include "core/EntryPoint.h"

#include "Test/TestLayer.h"

class GameApp : public FGEngine::Application
{
public:
	GameApp() : FGEngine::Application()
	{
		PushLayer(new TestLayer());
	}
};

FGEngine::Application* FGEngine::CreateApplication()
{
	return new GameApp();
}