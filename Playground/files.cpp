#include "shared_headers.h"
#include "files.h"

namespace IO
{
	Containers::Array<char> GetFileContent(LPCWSTR path)
	{
		HANDLE handle = CreateFile(path, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, 0);
		if (handle == 0)
		{
			return {};
		}

		Containers::Array<char> result;

		DWORD bytes_read;
		char buffer[1024];
		while (ReadFile(handle, static_cast<void*>(buffer), 1024, &bytes_read, nullptr) && bytes_read)
		{
			result.Append(buffer, bytes_read);
		}

		CloseHandle(handle);

		return result;
	}
}