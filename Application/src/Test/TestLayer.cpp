#include "TestLayer.h"
#include "FGEngine.h"


void TestLayer::OnUpdate(float deltaTime)
{
	//LogInfo("Test from application %f", 0.123f);

	//testDelegate->AddFunction(this, TestLayer::OnTestDelegated);
	//testDelegate->Broadcast(456, 321);
	//testDelegate->RemoveFunction(this, TestLayer::OnTestDelegated);
	//testDelegate->Broadcast(456, 321);
}

void TestLayer::OnTestDelegated(int val1, int val2)
{
	LogInfo("Test from delegate %d, %d", val1, val2);
}
