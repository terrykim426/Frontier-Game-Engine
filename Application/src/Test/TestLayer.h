#pragma once

#include "core/AppLayer.h"
#include "core/Delegate.h"

DECLARE_DELEGATE(Test, int, int);

class TestLayer : public FGEngine::AppLayer
{

public:
	virtual void OnUpdate(float deltaTime) override;

private:
	void OnTestDelegated(int val1, int val2);

private:
	TestDelegate testDelegate;
};

