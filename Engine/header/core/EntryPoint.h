#pragma once

#include "core/Application.h"

extern FGEngine::Application* FGEngine::CreateApplication();

int main(int argc, char** argv)
{
	auto* app = FGEngine::CreateApplication();
	app->Run();
	delete app;
}