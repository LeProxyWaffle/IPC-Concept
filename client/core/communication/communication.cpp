#pragma once
#include <core/communication/communication.h>

namespace server
{
    const std::wstring communication::c_interface::get_process_sid( std::uint32_t process_id )
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

    const std::uint32_t communication::c_interface::get_process_id( const std::wstring process_name )
    {
        HANDLE handle = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, NULL );
        DWORD procID = NULL;

        if ( handle == INVALID_HANDLE_VALUE )
            return procID;

        PROCESSENTRY32W entry = { 0 };
        entry.dwSize = sizeof( PROCESSENTRY32W );

        if ( Process32FirstW( handle, &entry ) )
        {
            if ( !_wcsicmp( process_name.data( ), entry.szExeFile ) )
            {
                procID = entry.th32ProcessID;
            }
            else while ( Process32NextW( handle, &entry ) )
            {
                if ( !_wcsicmp( process_name.data( ), entry.szExeFile ) )
                {
                    procID = entry.th32ProcessID;
                }
            }
        }

        CloseHandle( handle );
        return procID;
    }

    void communication::c_interface::send_request( request_data& request )
    {
        DWORD bytes_read = 0;

        request.is_client_running = true;

        if ( !WriteFile( ipc, &request, sizeof( request ), &bytes_read, NULL ) )
        {
            printf( " > Failed to write to pipe. Error: %lu\n", GetLastError( ) );
        }

        if ( !ReadFile( ipc, &request, sizeof( request ), &bytes_read, NULL ) )
        {
            printf( " > Failed to read from pipe. Error: %lu\n", GetLastError( ) );
        }
    }

	bool communication::c_interface::connect( const std::string module )
	{
        const auto process_pid = get_process_id( L"server.exe" );
        if ( !process_pid )
        {
            printf( " > Failed to get current process pid.\n" );
            return false;
        }

        const auto process_sid = get_process_sid( process_pid );
        if ( process_sid.empty( ) )
        {
            printf( " > Failed to get current process sid.\n" );
            return false;
        }

        const std::string sid( process_sid.begin( ), process_sid.end( ) );
        std::string sanitized_sid = sid;
        std::replace( sanitized_sid.begin( ), sanitized_sid.end( ), '\\', '_' );

        std::string pipe_name = "\\\\.\\pipe\\Local\\" + sanitized_sid + "_" + module;
        printf( " > Attempting to connect to pipe: %s\n", pipe_name.c_str( ) );

        return connect_to_pipe( pipe_name );
	}

	bool communication::c_interface::connect_to_pipe( const std::string& pipe_name )
	{
        ipc = CreateFileA(
            pipe_name.c_str( ),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
        );

        if ( ipc == INVALID_HANDLE_VALUE )
        {
            printf( " > Failed to connect to pipe. Error: %lu\n", GetLastError( ) );
            return false;
        }

        DWORD pipe_mode = PIPE_READMODE_MESSAGE;
        if ( !SetNamedPipeHandleState( ipc, &pipe_mode, NULL, NULL ) )
        {
            printf( " > Failed to set pipe mode. Error: %lu\n", GetLastError( ) );
            CloseHandle( ipc );
            ipc = INVALID_HANDLE_VALUE;
            return false;
        }

        printf( " > Successfully connected to pipe\n" );
        return true;
	}
}