#pragma once
#include <windows.h>
#include <list>
#include <string>
#include <array>
#include <vector>
#include <string.h>

using namespace std;


//�ṩ�����
//������װ��һ���飬ж��һ���飬���һ���飬ɾ��һ����
constexpr auto BLOCK_SIZE = 8192;
constexpr auto HEAD_SIZE = 2 * BLOCK_SIZE;

constexpr bool ISMOUNTED(const DWORD dwFlags) { return dwFlags & 2; }
constexpr bool ISLOADED(const DWORD dwFlags) { return dwFlags & 1; }
constexpr void SETMOUNTED(DWORD& dwFlags) { dwFlags |= 2; }
constexpr void SETLOADED(DWORD& dwFlags) { dwFlags |= 1; }
constexpr void CLRMOUNTED(DWORD& dwFlags) { dwFlags &= ~2; }
constexpr void CLRLOADED(DWORD& dwFlags) { dwFlags &= ~1; }

inline LARGE_INTEGER& operator<=(LARGE_INTEGER& L, const __int64& R)
{
	L.QuadPart = R;
	return L;
}

struct TBlock {
	TBlock()
	{
		this->hFile = INVALID_HANDLE_VALUE;
		this->hFileMap = INVALID_HANDLE_VALUE;
		this->lpRaw = 0;
		this->szPath = 0;
		this->dwFlags = 0;
	}
	TBlock(LPCWSTR _szPath)
	{
		this->hFile = INVALID_HANDLE_VALUE;
		this->hFileMap = INVALID_HANDLE_VALUE;
		this->lpRaw = 0;
		this->szPath = 0;
		this->dwFlags = 0;
		this->szPath = (LPCWSTR)_szPath;
	}

	LPCWSTR szPath;//unicode
	HANDLE hFile;
	HANDLE hFileMap;
	LPVOID lpRaw;
	DWORD Seq;
	DWORD dwFlags;
	//0_isloaded(0/1), 1_ismounted(0/1)
};

struct TFileChain {
	TFileChain() {
		this->hFileHead = INVALID_HANDLE_VALUE;
		this->hFileHeadMap = INVALID_HANDLE_VALUE;
		this->ChainHeadPath = nullptr;
		this->Chain.clear();
		this->OpeCount = 0;
		this->lpRaw = nullptr;
	}
	TFileChain(LPCWSTR _szPath)
	{
		this->hFileHead = INVALID_HANDLE_VALUE;
		this->hFileHeadMap = INVALID_HANDLE_VALUE;
		this->ChainHeadPath = nullptr;
		this->Chain.clear();
		this->OpeCount = 0;
		this->lpRaw = nullptr;
		this->ChainHeadPath = _szPath;
	}
	LPCWSTR ChainHeadPath;
	HANDLE hFileHead;
	HANDLE hFileHeadMap;
	LPVOID lpRaw;
	//�ļ�ͷ���ݣ���������dword��,/*���¼ƫ�ƣ�dword��*/=64n,���Ծ�ȣ�dword��,��λ��(LPCWSTR),...
	//��ʽ����ʼ4�ֽڿ�������
	list<TBlock* > Chain;
	DWORD OpeCount;
	CRITICAL_SECTION csOpe;//������
};

extern int LoadBlock(TBlock* _lpBlock);
extern int UnloadBlock(TBlock* _lpBlock);
extern int MountBlock(TBlock* _lpBlock);
extern int UnmountBlock(TBlock* _lpBlock);

extern int NewFileChain(TFileChain* _lpFileChain);
extern int InitFileChain(TFileChain* _lpFileChain);
extern int StartFileChain(TFileChain* _lpFileChain);
extern int StopFileChain(TFileChain* _lpFileChain);
extern int SaveFileChain(TFileChain* _lpFileChain);//critical section
extern int CloseFileChain(TFileChain* _lpFileChain);

extern int AddBlock2Chain(TFileChain* _lpFileChain, TBlock* _lpBlock);//critical section
extern int RmBlockFromChain(TFileChain* _lpFileChain, TBlock* _lpBlock);//critical section
extern int RmBlockFromChain(TFileChain* _lpFileChain, list<TBlock*>::iterator _lpBlock);//warning:no test!!//critical section