#include <core/communication/communication.h>

namespace server
{
	namespace exception
	{
		[[ nodiscard ]] long exception_filter(
			PEXCEPTION_POINTERS p_exception_pointers
		);
	}
}