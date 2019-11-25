#pragma once
#include <windows.h>
#include "FileBlocks.h"
#include <stdarg.h>
#include <ppl.h>
#include <queue>
#include <stack>
#include <vector>

using namespace std;

//constexpr auto MAX_STATIC_MOUNTED = 50;
//query, match, insert, delete, edit
//�洢�ṹ��ÿ����洢��������¼��ÿ����¼�п���Ψһ��ǣ�Mark��
//Flags
//��¼�����ǲ����ģ���¼�洢�ĸ�ʽΪ
//|Flags|Mark|Jump|Length|Record|...|Next
//| 2B  | 2B | 4B |  4B  |  ?B  |...|
////////////////////////////////////////////////////////////////////////////////////////////////
//����ͣ����һ�ָ���Ч�Ľṹ
//�洢һ����������
//ÿ����¼����һ����ͨ����
//ɾ���൱���������ϸ�
//ÿ���������ݣ���¼Mark��FLAGS�����������Ƿ�����¼���¼���������Ƿ�����¼���¼���������Ƿ����ڱ�����¼���������Ƿ����ڱ�����¼��\
//����ҪPIN����������ֻҪ��֤ÿ������GC�ϸ��Ժ��Դ���ԭ��������·���ϼ��ɣ���������Ȼ�ģ���Ϊ����������Ľṹ����
//��¼���ݣ�������

//0�����Mark��������¼������
//����ÿ���������������������ȡ��ȡ��������
//��ַ����������������֧���У��ڷ�֧������Mark

//ͬ����Ի��ĳ��Mark�������Ĵ�С����Mark=0��ͳ�Ƶ��ǿ������������ռ�
//����ռ価����������ƽ����Լ�С����ʱ��,����@@������ȱ���@@�����������@@@@@@@@

//�����ԣ���ʱ�����ǣ���Ϊ��ѯ���ǲ����ģ���д����Ҫ����
//////////////////////////////////////////////////////////////////////////////////////////////////
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//������������Ľṹ����ѯʱ�����ڵ�ָ�볯�����ƶ�һ��GRAIN
constexpr auto GRAIN_SIZE_IN_DWORD = 8;
constexpr auto TREE_SIZE_IN_DWORD = (8192 - 4) >> 2;
constexpr auto TREE_SIZE_IN_NODES = TREE_SIZE_IN_DWORD / GRAIN_SIZE_IN_DWORD;


constexpr DWORD EMPTY_MARK = 0;

struct TNode {
	TNode()
	{
		this->Head = 0;
	}
	union {
		DWORD Head;
		struct {
			WORD Mark;
			WORD Flags;
		};
	};
	DWORD Data[GRAIN_SIZE_IN_DWORD - 1];	//����8B
};

typedef DWORD TTrail;

struct TRecord {
	union {
		DWORD Head;
		struct {
			WORD Mark;
			WORD Unused;
		};
	};
	int SizeInNodes;
	void* lpRaw;
};

struct TRecordTree {
	const int MaxNodeCount = TREE_SIZE_IN_DWORD / GRAIN_SIZE_IN_DWORD;
	void* lpRaw;
	inline TNode* lpRoot() {
		return (TNode*)(this->lpRaw) - 1;
	}
};

struct TPtrRecord {
	TPtrRecord()
	{
		this->Tree = nullptr;
		this->Trail = nullptr;
		this->Mark = EMPTY_MARK;
	}
	TPtrRecord(DWORD _Mark)
	{
		this->Tree = nullptr;
		this->Trail = nullptr;
		this->Mark = _Mark;
	}
	TTrail* Trail;
	DWORD Mark;
	TRecordTree* Tree;
	TRecord& operator*();
	TRecord* operator->();
};

constexpr bool IFHOLDLEFTCHILD(const DWORD& _Head) { return (_Head & 0x00010000); };
constexpr bool IFHOLDRIGHTCHILD(const DWORD& _Head) { return (_Head & 0x00020000); };
constexpr bool IFHASLEFTCHILD(const DWORD& _Head) { return (_Head & 0x00100000); };
constexpr bool IFHASRIGHTCHILD(const DWORD& _Head) { return (_Head & 0x00200000); };

constexpr void SETHOLDLEFTCHILD(DWORD& _Head) { _Head |= 0x00010000; };
constexpr void SETHOLDRIGHTCHILD(DWORD& _Head) { _Head |= 0x00020000; };
constexpr void SETHASLEFTCHILD(DWORD& _Head) { _Head |= 0x00100000; };
constexpr void SETHASRIGHTCHILD(DWORD& _Head) { _Head |= 0x00200000; };

constexpr void CLRHOLDLEFTCHILD(DWORD& _Head) { _Head &= ~0x00010000; };
constexpr void CLRHOLDRIGHTCHILD(DWORD& _Head) { _Head &= ~0x00020000; };
constexpr void CLRHASLEFTCHILD(DWORD& _Head) { _Head &= ~0x00100000; };
constexpr void CLRHASRIGHTCHILD(DWORD& _Head) { _Head &= ~0x00200000; };

constexpr DWORD GETMARK(const DWORD& _Head) { return (DWORD)(_Head & 0x0000ffff); };

extern int GetTrailByMark(TRecordTree* _Tree, DWORD _Mark, TTrail* _Trail);//inner
extern int CompletePtr(TPtrRecord* _UncompletedPtr);

extern int SubAllocate(TRecordTree* _Tree, DWORD _Size, DWORD* _Index, list<int>* _List);
extern int InsertRecord(TRecord* _Record, DWORD _Index, TPtrRecord* _Ptr);
extern int DelRecord(TPtrRecord* _Ptr);
extern int CollectCertain(TPtrRecord* _Ptr);
extern int Collect(TRecordTree* _Tree);

