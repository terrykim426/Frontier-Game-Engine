#pragma once

#include "core/AppLayer.h"
#include "core/Delegate.h"

DECLARE_DELEGATE_ONE(Test, int);

class TestLayer : public FGEngine::AppLayer
{

public:
	virtual void OnUpdate(float deltaTime) override;

private:
	void OnTestDelegated(int abc);

private:
	DELEGATE_PTR(TestDelegate, testDelegate);
};

