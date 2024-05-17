#include <stdexcept>
#include <vector>
#include <typeindex>
#include <sstream>

namespace service_registry
{
	class DependencyLoopException : public std::runtime_error
	{
	public:
		explicit DependencyLoopException(std::vector<std::type_index> dependencyPath)
			: std::runtime_error{ CreatePathString(dependencyPath) }
		{
		}

	private:
		std::string CreatePathString(const std::vector<std::type_index>& dependencyPath) const
		{
			// Avoid throwing exceptions in an exception
			// String streams don't throw exceptions by default
			// bad alloc exception can still be thrown, don't handle this, program should be terminated when this happens
			std::stringstream pathStream{};
			for (size_t i{}; i < dependencyPath.size(); ++i)
			{
				pathStream << dependencyPath[i].name();
				if (i < dependencyPath.size() - 1)
					pathStream << ", ";
			}

			return pathStream.str();
		}
	};
}