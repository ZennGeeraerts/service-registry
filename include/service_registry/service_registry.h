#include "service.h"
#include "dependency_loop_exception.h"

#include <type_traits>
#include <unordered_map>
#include <deque>
#include <mutex>

namespace service_registry
{
	class ServiceRegistry final
	{
	public:
		ServiceRegistry() = default;
		~ServiceRegistry();

		template<typename Type>
		void Register()
		{
			static_assert(!IsDecorated<Type>(), "Type must be undecorated.");
			static_assert(std::is_default_constructible_v<Type> || std::is_constructible_v<Type, ServiceRegistry&>,
				"Type must be default-constructable or registry-constructable.");

			std::lock_guard lock{ m_Mutex };
			auto it = m_ServiceMap.find(typeid(Type));
			if (it != m_ServiceMap.end())
				return;

			Service service{};
			service.Constructor = [this]() mutable -> void*
				{
					if constexpr (std::is_constructible_v<Type, ServiceRegistry&>)
						return new Type(*this);
					else
						return new Type();
				};
			service.Destructor = [](void* pInstance)
				{
					delete static_cast<Type*>(pInstance);
					pInstance = nullptr;
				};

			m_ServiceMap.emplace(typeid(Type), std::move(service));
		}

		template<typename InterfaceType, typename ConcreteType>
		void Register()
		{
			static_assert(!IsDecorated<InterfaceType>(), "Interface type must be undecorated.");
			static_assert(!IsDecorated<ConcreteType>(), "Concrete type must be undecorated.");
			static_assert(std::is_base_of_v<InterfaceType, ConcreteType>, "Concrete type must derive from interface.");
			static_assert(std::is_default_constructible_v<ConcreteType> || std::is_constructible_v<ConcreteType, ServiceRegistry&>,
				"Concrete type must be default-constructable or registry-constructable.");

			std::lock_guard lock{ m_Mutex };
			auto it = m_ServiceMap.find(typeid(InterfaceType));
			if (it != m_ServiceMap.end())
				return;

			Service service{};
			service.Constructor = [this]() mutable -> void*
				{
					if constexpr (std::is_constructible_v<ConcreteType, ServiceRegistry&>)
						return static_cast<InterfaceType*>(new ConcreteType(*this));
					else
						return static_cast<InterfaceType*>(new ConcreteType());
				};
			service.Destructor = [](void* pInstance)
				{
					delete static_cast<InterfaceType*>(pInstance);
					pInstance = nullptr;
				};

			m_ServiceMap.emplace(typeid(InterfaceType), std::move(service));
		}

		template<typename InterfaceType, typename Factory>
		void Register(Factory factory)
		{
			static_assert(!IsDecorated<InterfaceType>(), "Interface type must be undecorated.");
			static_assert(std::is_invocable_r_v<std::unique_ptr<InterfaceType>, Factory>, "Factory must be a parameter-less callable that generates interfaces of InterfaceType.");

			std::lock_guard lock{ m_Mutex };
			auto it = m_ServiceMap.find(typeid(InterfaceType));
			if (it != m_ServiceMap.end())
				return;

			Service service{};
			service.Constructor = [factory = std::move(factory)]() mutable -> void*
				{
					return factory().release();
				};
			service.Destructor = [](void* pInstance)
				{
					delete static_cast<InterfaceType*>(pInstance);
					pInstance = nullptr;
				};

			m_ServiceMap.emplace(typeid(InterfaceType), std::move(service));
		}

		template<typename Type>
		void Unregister()
		{
			static_assert(!IsDecorated<Type>(), "Type must be undecorated.");

			std::lock_guard lock{ m_Mutex };
			auto it = m_ServiceMap.find(typeid(Type));
			if (it == m_ServiceMap.end())
				return;

			Unregister(it->first);
		}

		template <typename Type>
		Type* Get()
		{
			static_assert(!IsDecorated<Type>(), "Type must be undecorated.");

			std::lock_guard lock{ m_Mutex };
			auto it = m_ServiceMap.find(typeid(Type));
			if (it == m_ServiceMap.end())
				return nullptr;

			AddDependency(it->first, it->second);
			CreateInstance(it->first, it->second);

			return static_cast<Type*>(it->second.pInstance);
		}

	private:
		template <typename Type>
		static constexpr bool IsDecorated()
		{
			return std::is_reference_v<Type> || std::is_pointer_v<Type> || std::is_const_v<Type> || std::is_volatile_v<Type>;
		}

		void CreateInstance(std::type_index serviceType, Service& service);
		void AddDependency(std::type_index serviceType, Service& service) const;
		void Unregister(std::type_index serviceType);
		void RemoveDependency(std::type_index serviceType);

	private:
		mutable std::recursive_mutex m_Mutex;
		std::unordered_map<std::type_index, Service> m_ServiceMap;
		std::deque<std::pair<std::type_index, Service*>> m_CtorPath;
	};
}