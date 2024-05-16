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
		std::string CreatePathString(const std::vector<std::type_index>& dependencyPath)
		{
			std::stringstream pathStream;
			for (const auto& dependency : dependencyPath)
			{
				pathStream << dependency.name();
				if (dependency != dependencyPath.back())
					pathStream << ", ";
			}

			m_PathString = pathStream.str();
			return m_PathString;
		}

	private:
		// Store to be accessible for the base class
		std::string m_PathString;
	};
}