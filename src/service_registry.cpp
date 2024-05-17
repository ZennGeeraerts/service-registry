#include "service_registry/service_registry.h"

#include <algorithm>

namespace service_registry
{
	ServiceRegistry::~ServiceRegistry()
	{
		// First remove services with no dependencies
		// Remove the dependencies on these removed services in other services
		// Keep iterating until all services are removed
		while (!m_ServiceMap.empty())
		{
			for (auto it = m_ServiceMap.cbegin(); it != m_ServiceMap.cend(); )
			{
				if (it->second.Dependencies.empty())
				{
					RemoveDependency(it->first);
					it = m_ServiceMap.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
	}

	void ServiceRegistry::CreateInstance(std::type_index serviceType, Service& service)
	{
		if (service.pInstance)
			return;

		m_CtorPath.push_back(std::make_pair(serviceType, &service));
		service.pInstance = service.Constructor();
		m_CtorPath.pop_back();
	}

	void ServiceRegistry::AddDependency(std::type_index serviceType, Service& service) const
	{
		auto ctorEntry = std::make_pair(serviceType, &service);
		auto it = std::find(m_CtorPath.begin(), m_CtorPath.end(), ctorEntry);
		if (it != m_CtorPath.end())
		{
			std::vector<std::type_index> dependencyPath{};
			for (const auto& entry : m_CtorPath)
				dependencyPath.emplace_back(entry.first);
			dependencyPath.emplace_back(it->first);

			throw DependencyLoopException{ dependencyPath };
		}

		if (!m_CtorPath.empty())
		{
			const auto& parent = m_CtorPath.back();
			parent.second->Dependencies.insert(serviceType);
		}
	}

	void ServiceRegistry::Unregister(std::type_index serviceType)
	{
		m_ServiceMap.erase(serviceType);
		RemoveDependency(serviceType);
	}

	void ServiceRegistry::RemoveDependency(std::type_index serviceType)
	{
		// Remove dependency on this unregistered service in other services
		for (auto& it : m_ServiceMap)
		{
			auto dependencyIt = it.second.Dependencies.find(serviceType);
			if (dependencyIt != it.second.Dependencies.end())
				it.second.Dependencies.erase(dependencyIt);
		}
	}
}