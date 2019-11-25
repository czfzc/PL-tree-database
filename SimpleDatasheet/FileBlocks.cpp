#include "FileBlocks.h"


int LoadBlock(TBlock* _lpBlock)
{
	if (ISLOADED(_lpBlock->dwFlags))
		return 0;
	_lpBlock->hFile = CreateFile(_lpBlock->szPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, \
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (_lpBlock->hFile == INVALID_HANDLE_VALUE)
	{
		_asm int 3
		return -1;
	}
	_lpBlock->hFileMap = CreateFileMapping(_lpBlock->hFile, NULL, PAGE_READWRITE, 0, BLOCK_SIZE, NULL);
	if (_lpBlock->hFileMap == INVALID_HANDLE_VALUE)
	{
		_asm int 3
		return -1;
	}
	SETLOADED(_lpBlock->dwFlags);
	LARGE_INTEGER temp;
	temp <= 0;
	SetFilePointerEx(_lpBlock->hFile, temp, nullptr, FILE_BEGIN);
	if (!ReadFile(_lpBlock->hFile, &_lpBlock->Seq, sizeof(DWORD), new DWORD, nullptr))
	{
		_asm int 3
		return -1;
	}
	return 0;
}

int UnloadBlock(TBlock* _lpBlock)
{
	if (!ISLOADED(_lpBlock->dwFlags))
		return -1;
	LARGE_INTEGER temp;
	temp <= 0;
	SetFilePointerEx(_lpBlock->hFile, temp, nullptr, FILE_BEGIN);
	if (!WriteFile(_lpBlock->hFile, &_lpBlock->Seq, sizeof(DWORD), new DWORD, nullptr))
	{
		_asm int 3
		return -1;
	}
	CloseHandle(_lpBlock->hFileMap);
	CloseHandle(_lpBlock->hFile);
	CLRLOADED(_lpBlock->dwFlags);
	return 0;
}

int MountBlock(TBlock* _lpBlock)
{
	if (ISMOUNTED(_lpBlock->dwFlags))
		return 0;
	_lpBlock->lpRaw = (DWORD*)MapViewOfFile(_lpBlock->hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, BLOCK_SIZE) + 1;//+1 Ìø¹ýBlockÍ·
	SETMOUNTED(_lpBlock->dwFlags);
	if (_lpBlock->lpRaw <= 0)
	{
		_asm int 3
		return -1;
	}
	return 0;
}

int UnmountBlock(TBlock* _lpBlock)
{
	if (!ISMOUNTED(_lpBlock->dwFlags))
		return -1;
	int ret = UnmapViewOfFile((LPVOID)((DWORD*)_lpBlock->lpRaw + 1));
	CLRMOUNTED(_lpBlock->dwFlags);
	_lpBlock->lpRaw = nullptr;
	if (ret)
		return 0;
	_asm int 3
	return -1;
}

int NewFileChain(TFileChain* _lpFileChain)
{
	memset(_lpFileChain->lpRaw, 0, HEAD_SIZE);
	return 0;
}

int InitFileChain(TFileChain* _lpFileChain)
{
	InitializeCriticalSection(&_lpFileChain->csOpe);
	_lpFileChain->hFileHead = CreateFile(_lpFileChain->ChainHeadPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, \
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (_lpFileChain->hFileHead == INVALID_HANDLE_VALUE)
	{
		_asm int 3
		return -1;
	}
	_lpFileChain->hFileHeadMap = CreateFileMapping(_lpFileChain->hFileHead, NULL, PAGE_READWRITE, 0, HEAD_SIZE, NULL);
	if (_lpFileChain->hFileHeadMap == INVALID_HANDLE_VALUE || !(_lpFileChain->hFileHeadMap))
	{
		_asm int 3
		return -1;
	}
	_lpFileChain->lpRaw = MapViewOfFile(_lpFileChain->hFileHeadMap, FILE_MAP_ALL_ACCESS, 0, 0, HEAD_SIZE);
	if (_lpFileChain->lpRaw <= 0)
	{
		_asm int 3
		return -1;
	}
	return 0;
}

int StartFileChain(TFileChain* _lpFileChain)
{
	auto ptr = _lpFileChain->lpRaw;
	DWORD _count = *(DWORD*)ptr;
	if (_count <= 0)
		return -1;
	auto ptrwstr = (DWORD*)ptr + 1;
	for (size_t i = 0; i < _count; i++)
		_lpFileChain->Chain.push_back(new TBlock((WCHAR*)ptrwstr + (i << 6)));

	for (auto& p : _lpFileChain->Chain)
		LoadBlock(p);
	return 0;
}

int SaveFileChain(TFileChain* _lpFileChain)
{
	memset(_lpFileChain->lpRaw, 0, HEAD_SIZE);
	auto ptr = _lpFileChain->lpRaw;
	EnterCriticalSection(&_lpFileChain->csOpe);//protect
	DWORD _count = _lpFileChain->Chain.size();
	*(DWORD*)ptr = _count;
	auto ptrwstr = (DWORD*)ptr + 1;
	size_t i = 0;
	size_t offset = 0;
	for (auto p = _lpFileChain->Chain.begin(); p != _lpFileChain->Chain.end(); p++)
		wcsncpy_s((wchar_t*)ptrwstr + (i++ << 6), 64, (*p)->szPath, 64);
	LeaveCriticalSection(&_lpFileChain->csOpe);
	return 0; //
}

int AddBlock2Chain(TFileChain* _lpFileChain, TBlock* _lpBlock)
{
	if (!_lpBlock)
		return -1;
	EnterCriticalSection(&_lpFileChain->csOpe);
	_lpFileChain->Chain.push_back(_lpBlock);
	LeaveCriticalSection(&_lpFileChain->csOpe);
	LoadBlock(_lpBlock);
	MountBlock(_lpBlock);
	return 0;
}

int RmBlockFromChain(TFileChain* _lpFileChain, TBlock* _lpBlock)
{
	EnterCriticalSection(&_lpFileChain->csOpe);
	_lpFileChain->Chain.remove(_lpBlock);
	LeaveCriticalSection(&_lpFileChain->csOpe);
	UnloadBlock(_lpBlock);
	return 0;
}

int RmBlockFromChain(TFileChain* _lpFileChain, list<TBlock*>::iterator _lpBlock) //no check!!
{
	EnterCriticalSection(&_lpFileChain->csOpe);
	UnloadBlock(*_lpBlock);
	_lpFileChain->Chain.erase(_lpBlock);
	LeaveCriticalSection(&_lpFileChain->csOpe);
	return 0;
}

int StopFileChain(TFileChain* _lpFileChain)
{
	SaveFileChain(_lpFileChain);
	for (auto& p : _lpFileChain->Chain)
	{
		UnmountBlock(p);
		UnloadBlock(p);
	}
	return 0;
}

int CloseFileChain(TFileChain* _lpFileChain)
{
	int ret = UnmapViewOfFile(_lpFileChain->lpRaw);
	if (!ret)
	{
		_asm int 3
		return -1;
	}
	CloseHandle(_lpFileChain->hFileHeadMap);
	CloseHandle(_lpFileChain->hFileHead);
	DeleteCriticalSection(&_lpFileChain->csOpe);
	return 0;
}