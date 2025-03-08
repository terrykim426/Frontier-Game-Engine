#pragma once
#include "subsystem/EngineSubsystem.h"
#include "renderer/RendererProperties.h"

#include <memory>

namespace FGEngine
{
class RendererSubsystem : public EngineSubsystem
{
public:
	explicit RendererSubsystem(const RendererProperties& rendererProperties = RendererProperties());
	virtual ~RendererSubsystem();

	void SetClearColor(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
	void Clear();

	void Render();

	void Resize(const std::shared_ptr<class IWindowEvent>& windowEvent);

private:
	std::unique_ptr<class IRendererAPI> api;
};
}

