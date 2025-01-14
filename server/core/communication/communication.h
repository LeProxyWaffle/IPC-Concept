#include <core/dependencies/utils/utils.h>

namespace server
{
	namespace data
	{
		typedef struct _request_data
		{
			bool is_client_running;
			bool is_operation_completed;

			bool is_process_running;
		} request_data, * prequest_data;
	}

	class c_interface
	{
	public:
		const std::wstring get_process_sid(
			std::uint32_t process_id );
		const std::uint32_t get_process_id( const std::wstring process_name );
		void send_cmd( data::request_data& request );

		bool create_buffer(
			const std::string buffer );

		bool create( const std::string module );
	private:
		HANDLE ipc = nullptr;
	};
}