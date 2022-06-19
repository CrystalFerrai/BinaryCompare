// Copyright 2022 Crystal Ferrai
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <Windows.h>

// Windows-only fast binary file comparison
// Return value:
//   0  Files are not identical
//   1  Files are identical
//  -1  An error occured while trying to read files
extern "C" __declspec(dllexport) int CompareFiles(const wchar_t* aPath, const wchar_t* bPath)
{
	// If the paths are the same, then the files are identical
	if (wcscmp(aPath, bPath) == 0) return 1;

	// Share write access
	HANDLE aHandle = CreateFileW(aPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (aHandle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	HANDLE bHandle = CreateFileW(bPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (bHandle == INVALID_HANDLE_VALUE)
	{
		CloseHandle(aHandle);
		return -1;
	}

	LARGE_INTEGER aSizeStruct;
	LARGE_INTEGER bSizeStruct;
	GetFileSizeEx(aHandle, &aSizeStruct);
	GetFileSizeEx(bHandle, &bSizeStruct);
	UINT64 sourceSize = *(UINT64*)& aSizeStruct;
	UINT64 lastBackupSize = *(UINT64*)& bSizeStruct;

	bool foundDifference = sourceSize != lastBackupSize;
	if (!foundDifference)
	{
		// Files are the same size, need to binary compare
		HANDLE aMapHandle = CreateFileMappingW(aHandle, NULL, PAGE_READONLY, 0u, 0u, NULL);
		if (!aMapHandle)
		{
			CloseHandle(aHandle);
			CloseHandle(bHandle);
			return -1;
		}
		HANDLE bMapHandle = CreateFileMappingW(bHandle, NULL, PAGE_READONLY, 0u, 0u, NULL);
		if (!bMapHandle)
		{
			CloseHandle(aHandle);
			CloseHandle(bHandle);
			CloseHandle(aMapHandle);
			return -1;
		}

		// Compare in chunks to limit memory consumption. Max allocation will be approximately ChunkSize * 2.
		const static DWORD ChunkSize = 1024 * 1024 * 256;

		for (UINT64 offset = 0; !foundDifference && offset < sourceSize; offset += ChunkSize)
		{
			DWORD* offsetLow = (DWORD*)& offset;
			DWORD* offsetHigh = offsetLow + 1;
			DWORD numBytes = ChunkSize;
			if (offset + ChunkSize > sourceSize)
			{
				numBytes = (DWORD)(sourceSize - offset);
			}

			void* aData = MapViewOfFile(aMapHandle, FILE_MAP_READ, *offsetHigh, *offsetLow, numBytes);
			if (!aData)
			{
				CloseHandle(aHandle);
				CloseHandle(bHandle);
				CloseHandle(aMapHandle);
				CloseHandle(bMapHandle);
				return -1;
			}
			void* bData = MapViewOfFile(bMapHandle, FILE_MAP_READ, *offsetHigh, *offsetLow, numBytes);
			if (!bData)
			{
				UnmapViewOfFile(aData);
				CloseHandle(aData);
				CloseHandle(aHandle);
				CloseHandle(bHandle);
				CloseHandle(aMapHandle);
				CloseHandle(bMapHandle);
				return -1;
			}

			if (memcmp(aData, bData, numBytes) != 0)
			{
				foundDifference = true;
				break;
			}

			UnmapViewOfFile(aData);
			UnmapViewOfFile(bData);
		}

		CloseHandle(aMapHandle);
		CloseHandle(bMapHandle);
	}
	CloseHandle(aHandle);
	CloseHandle(bHandle);

	return foundDifference ? 0 : 1;
}