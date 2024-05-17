#include <catch/catch.hpp>
#include <service_registry/service_registry.h>
#include <iostream>

namespace service_registry
{
	class ChildService final
	{
	public:
		ChildService() = default;

		int GetData() const
		{
			return m_Data;
		}

	private:
		int m_Data = 12;
	};

	class TestService final
	{
	public:
		TestService(ServiceRegistry& registry)
		{
			m_pChildService = registry.Get<ChildService>();
			m_Data = m_pChildService->GetData();
		}

		void Test()
		{
			m_Data = m_pChildService->GetData() + 1;
		}

		int GetData() const
		{
			return m_Data;
		}

	private:
		int m_Data = 0;
		ChildService* m_pChildService = nullptr;
	};

	class Food
	{
	public:
		Food() = default;
		virtual ~Food() = default;
		virtual std::string Eat() = 0;
	};

	class Banana : public Food
	{
	public:
		Banana() = default;
		~Banana() = default;

		std::string Eat() override
		{
			return "Eating a banana";
		}
	};

	class Pizza : public Food
	{
	public:
		Pizza() = default;
		~Pizza() = default;

		std::string Eat() override
		{
			return "Eating pizza";
		}
	};

	class EngineService final
	{
	public:
		EngineService() = default;
	};

	class CarService;

	class TireService final
	{
	public:
		TireService(ServiceRegistry& registry)
		{
			m_pCarService = registry.Get<CarService>();
		}

		CarService* m_pCarService;
	};

	class WheelService final
	{
	public:
		WheelService(ServiceRegistry& registry)
		{
			m_pTireService = registry.Get<TireService>();
		}

	private:
		TireService* m_pTireService;
	};

	class CarService final
	{
	public:
		CarService(ServiceRegistry& registry)
		{
			m_pEngineService = registry.Get<EngineService>();
			m_pWheelService = registry.Get<WheelService>();
		}

	private:
		EngineService* m_pEngineService;
		WheelService* m_pWheelService;
	};

	TEST_CASE("ServiceRegistry")
	{
		SECTION("Register")
		{
			auto registry = ServiceRegistry{};
			registry.Register<TestService>();
			registry.Register<ChildService>();
			auto pTestService = registry.Get<TestService>();

			REQUIRE(pTestService->GetData() == 12);

			pTestService->Test();

			REQUIRE(pTestService->GetData() == 13);
		}

		SECTION("Register Interface")
		{
			auto registry = ServiceRegistry{};
			registry.Register<Food, Banana>();
			auto pFood = registry.Get<Food>();

			REQUIRE(pFood->Eat() == "Eating a banana");

			registry.Unregister<Food>();
			registry.Register<Food, Pizza>();
			pFood = registry.Get<Food>();

			REQUIRE(pFood->Eat() == "Eating pizza");
		}

		SECTION("Register Factory")
		{
			auto registry = ServiceRegistry{};
			registry.Register<Food>([]()
				{
					return std::make_unique<Banana>();
				});
			auto pFood = registry.Get<Food>();

			REQUIRE(pFood->Eat() == "Eating a banana");

			registry.Unregister<Food>();
			registry.Register<Food>([]()
				{
					return std::make_unique<Pizza>();
				});
			pFood = registry.Get<Food>();

			REQUIRE(pFood->Eat() == "Eating pizza");
		}

		SECTION("Circular Dependencies")
		{
			auto registry = ServiceRegistry{};
			registry.Register<CarService>();
			registry.Register<WheelService>();
			registry.Register<TireService>();
			registry.Register<EngineService>();

			REQUIRE_THROWS_AS(registry.Get<CarService>(), DependencyLoopException);

			try
			{
				registry.Get<CarService>();
			}
			catch (const DependencyLoopException& exc)
			{
				std::cout << exc.what() << '\n';
			}
		}
	}
}