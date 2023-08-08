#include "pch.h"
#include "core/AppLayerStack.h"
#include "core/AppLayer.h"

namespace FGEngine
{
	AppLayerStack::~AppLayerStack()
	{
		ClearAllLayers();
	}

	void AppLayerStack::PushLayer(AppLayer* appLayer)
	{
		if (appLayer)
		{
			appLayer->OnAttach();
			layers.emplace(layers.begin() + layerInsertIndex, appLayer);
			layerInsertIndex++;
		}
	}

	void AppLayerStack::PushOverlay(AppLayer* appLayer)
	{
		if (appLayer)
		{
			appLayer->OnAttach();
			layers.emplace_back(appLayer);
		}
	}

	void AppLayerStack::PopLayer(AppLayer* appLayer)
	{
		if (appLayer)
		{
			auto it = std::find(layers.begin(), layers.begin() + layerInsertIndex, appLayer);
			if (it != layers.end())
			{
				appLayer->OnDetach();
				layers.erase(it);
				layerInsertIndex--;
			}
		}
	}

	void AppLayerStack::PopOverlay(AppLayer* appLayer)
	{
		if (appLayer)
		{
			auto it = std::find(layers.begin() + layerInsertIndex, layers.end(), appLayer);
			if (it != layers.end())
			{
				appLayer->OnDetach();
				layers.erase(it);
			}
		}
	}

	void AppLayerStack::ClearAllLayers()
	{
		for (AppLayer* layer : layers)
		{
			layer->OnDetach();
			delete layer;
		}
		layers.clear();
		layerInsertIndex = 0;
	}
}