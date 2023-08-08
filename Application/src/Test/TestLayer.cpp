#include "TestLayer.h"
#include "FGEngine.h"


void TestLayer::OnUpdate(float deltaTime)
{
	LogInfo("Test from application %f", 0.123f);

	FGEngine::DelegateHandle handle = testDelegate->Add1(this, TestLayer::OnTestDelegated);
	testDelegate->Broadcast(456);
	testDelegate->Remove(handle);
	testDelegate->Broadcast(456);
}

void TestLayer::OnTestDelegated(int abc)
{
	LogInfo("Test from delegate %d", abc);
}
