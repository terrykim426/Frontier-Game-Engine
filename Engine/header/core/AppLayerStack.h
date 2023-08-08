#pragma once

#include "core/Core.h"
#include <vector>

namespace FGEngine
{
	class AppLayer;

	class ENGINE_API AppLayerStack
	{
	public:
		AppLayerStack() = default;
		~AppLayerStack();

		void PushLayer(AppLayer* appLayer);
		void PushOverlay(AppLayer* appLayer);
		void PopLayer(AppLayer* appLayer);
		void PopOverlay(AppLayer* appLayer);

		void ClearAllLayers();

		std::vector<AppLayer*>::iterator begin() { return layers.begin(); }
		std::vector<AppLayer*>::iterator end() { return layers.end(); }
		std::vector<AppLayer*>::reverse_iterator rbegin() { return layers.rbegin(); }
		std::vector<AppLayer*>::reverse_iterator rend() { return layers.rend(); }

		std::vector<AppLayer*>::const_iterator begin() const { return layers.begin(); }
		std::vector<AppLayer*>::const_iterator end() const { return layers.end(); }
		std::vector<AppLayer*>::const_reverse_iterator rbegin() const { return layers.rbegin(); }
		std::vector<AppLayer*>::const_reverse_iterator rend() const { return layers.rend(); }

	private:
#pragma warning (suppress : 4251) // need to find out how to resolve this warning correctly
		std::vector<AppLayer*> layers;
		unsigned int layerInsertIndex = 0;
	};
}