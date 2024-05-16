#include <catch/catch.hpp>
#include <service_registry/service_registry.h>

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

		// TODO: Test circular dependencies
	}
}