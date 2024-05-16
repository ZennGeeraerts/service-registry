#include <unordered_set>
#include <typeindex>
#include <functional>

namespace service_registry
{
	struct Service final
	{
		Service() = default;
		~Service()
		{
			if (Destructor && pInstance)
				Destructor(pInstance);
		}

		std::unordered_set<std::type_index> Dependencies;

		std::function<void* ()> Constructor;
		std::function<void(void*)> Destructor;
		void* pInstance;
	};
}