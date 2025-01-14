#include <core/linkers/stdafx.h>
#include <core/linkers/exception.h>

using namespace server;

int __cdecl main( int argc, char** argv )
{
	SetConsoleTitleA( "Riot Client" );
	SetUnhandledExceptionFilter( exception::exception_filter );

	communication::c_interface communication;
	if ( !communication.connect( "Riot" ) )
	{
		printf( " > failed to setup server" );
		return std::getchar( );
	}

	communication::request_data data;
	communication.send_request( data );

	printf( " > sent request...\n" );
	return std::getchar( );
}