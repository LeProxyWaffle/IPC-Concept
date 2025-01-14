#include <core/linkers/stdafx.h>
#include <core/linkers/exception.h>

using namespace server;

int __cdecl main( int argc , char** argv )
{
	SetConsoleTitleA( encrypt( "Riot Server" ) );
	SetUnhandledExceptionFilter( exception::exception_filter );
	if ( !utils::enable_privilege( encrypt( L"SeDebugPrivilege" ).decrypt( ) ) ) {
		printf( encrypt( "failed to enable debug privilege.\n" ) );
		return std::getchar( );
	}

	printf( encrypt( "initializing Riot Server...\n" ) );
	printf( encrypt( "=== Riot Execution :: server_interface::c_interface::create() ===\n" ) );
	printf( encrypt( "...................................................................\n" ) );
	Sleep( 250 );

	server::c_interface shared_memory { };
	if ( !shared_memory.create( encrypt( "Riot" ).decrypt( ) ) ) {
		printf( encrypt( " > failed to create shared memory (error : %lu).\n" ) , GetLastError( ) );
		return std::getchar( );
	}

	printf( " > waiting for request...\n" );

	server::data::request_data ping_request { };
	ping_request.is_process_running = true;
	shared_memory.send_cmd( ping_request );
	if ( !ping_request.is_client_running ) {
		printf( encrypt( " > client process ping was not successful.\n" ) );
		return std::getchar( );
	}

	printf( " > recieved request : %i\n" , ping_request.is_client_running );

	return std::getchar( );
}