#include <core/linkers/stdafx.h>

namespace server
{
	namespace utils
	{
		[[ nodiscard ]] bool enable_privilege(
			const std::wstring& privilege_name
		) {
			HANDLE token_handle = nullptr;
			if ( !OpenProcessToken( GetCurrentProcess( ) , TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY , &token_handle ) )
				return false;

			LUID luid{};
			if ( !LookupPrivilegeValueW( nullptr , privilege_name.data( ) , &luid ) )
				return false;

			TOKEN_PRIVILEGES token_state{};
			token_state.PrivilegeCount = 1;
			token_state.Privileges[ 0 ].Luid = luid;
			token_state.Privileges[ 0 ].Attributes = SE_PRIVILEGE_ENABLED;

			if ( !AdjustTokenPrivileges( token_handle , FALSE , &token_state , sizeof( TOKEN_PRIVILEGES ) , nullptr , nullptr ) )
				return false;

			CloseHandle( token_handle );
			return true;
		}
	}
}