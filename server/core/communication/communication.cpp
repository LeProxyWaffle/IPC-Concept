#include <core/communication/communication.h>

namespace server
{
	const std::wstring c_interface::get_process_sid( std::uint32_t process_id )
	{
		HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION, FALSE, process_id );
		if ( hProcess == NULL )
		{
			return L"";
		}

		HANDLE hToken;
		if ( !OpenProcessToken( hProcess, TOKEN_QUERY, &hToken ) )
		{
			CloseHandle( hProcess );
			return L"";
		}

		DWORD tokenInfoLength = 0;
		if ( !GetTokenInformation( hToken, TokenUser, NULL, 0, &tokenInfoLength ) &&
			GetLastError( ) != ERROR_INSUFFICIENT_BUFFER )
		{
			CloseHandle( hToken );
			CloseHandle( hProcess );
			return L"";
		}

		PTOKEN_USER pTokenUser = reinterpret_cast< PTOKEN_USER >( new BYTE[ tokenInfoLength ] );
		if ( !GetTokenInformation( hToken, TokenUser, pTokenUser, tokenInfoLength, &tokenInfoLength ) )
		{
			delete[ ] reinterpret_cast< BYTE* >( pTokenUser );
			CloseHandle( hToken );
			CloseHandle( hProcess );
			return L"";
		}

		LPWSTR pSidString;
		if ( !ConvertSidToStringSidW( pTokenUser->User.Sid, &pSidString ) )
		{
			delete[ ] reinterpret_cast< BYTE* >( pTokenUser );
			CloseHandle( hToken );
			CloseHandle( hProcess );
			return L"";
		}

		std::wstring userSID( pSidString );
		LocalFree( pSidString );
		delete[ ] reinterpret_cast< BYTE* >( pTokenUser );

		return userSID;
	}

	void c_interface::send_cmd( data::request_data& request )
	{
		DWORD bytes_written = 0;
		DWORD bytes_read = 0;

		if ( !ConnectNamedPipe( ipc, NULL ) && GetLastError( ) != ERROR_PIPE_CONNECTED )
		{
			printf( encrypt( " > Waiting for client connection...\n" ) );
			return;
		}

		request.is_operation_completed = true;

		if ( !WriteFile( ipc, &request, sizeof( request ), &bytes_written, NULL ) )
		{
			DWORD error = GetLastError( );
			printf( encrypt( " > Failed to write to pipe. Error: %lu\n" ), error );
			return;
		}

		if ( !ReadFile( ipc, &request, sizeof( request ), &bytes_read, NULL ) )
		{
			DWORD error = GetLastError( );
			printf( encrypt( " > Failed to read from pipe. Error: %lu\n" ), error );
			return;
		}

		if ( !request.is_client_running )
		{
			printf( encrypt( " > client is not running\n" ) );
			return;
		}
	}

	bool c_interface::create_buffer( const std::string buffer )
	{
		ipc = CreateNamedPipeA(
			buffer.c_str( ),
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE |
			PIPE_READMODE_MESSAGE |
			PIPE_WAIT,
			1,
			4096,
			4096,
			0,
			NULL
		);

		return ipc != INVALID_HANDLE_VALUE;
	}

	bool c_interface::create( const std::string module )
	{
		const auto process_sid = get_process_sid( GetCurrentProcessId( ) );
		if ( process_sid.empty( ) )
		{
			printf( encrypt( " > failed to get current process sid.\n" ) );
			return false;
		}

		const std::string sid( process_sid.begin( ), process_sid.end( ) );

		std::string sanitized_sid = sid;
		std::replace( sanitized_sid.begin( ), sanitized_sid.end( ), '\\', '_' );

		std::string pipe_name = "\\\\.\\pipe\\Local\\" + sanitized_sid + "_" + module;
		printf( encrypt( " > creating named pipe: %s\n" ), pipe_name.c_str( ) );

		return create_buffer( pipe_name );
	}
}