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
//存储结构：每个块存储若干条记录，每条记录有块内唯一标记（Mark）
//Flags
//记录长度是不定的，记录存储的格式为
//|Flags|Mark|Jump|Length|Record|...|Next
//| 2B  | 2B | 4B |  4B  |  ?B  |...|
////////////////////////////////////////////////////////////////////////////////////////////////
//！！停！有一种更高效的结构
//存储一棵满二叉树
//每条记录构成一个连通子树
//删除相当于子树的上浮
//每个结点的数据：记录Mark，FLAGS：（左子树是否存在下级记录，右子树是否存在下级记录，左子树是否属于本级记录，右子树是否属于本级记录）\
//不需要PIN！！！！！只要保证每棵子树GC上浮以后仍处在原来的搜索路径上即可！！这是显然的，因为二叉树本身的结构所限
//记录数据（定长）

//0代表空Mark，本级记录不存在
//对于每棵子树，做中序遍历即可取得取得树内容
//地址（索引）：遍历分支序列，在分支上搜索Mark

//同理可以获得某个Mark的子树的大小，若Mark=0则统计的是空余子树连续空间
//分配空间尽可能倾向于平衡可以减小搜索时间,故作@@广度优先遍历@@而非中序遍历@@@@@@@@

//并发性：暂时不考虑，认为查询都是并发的，而写入需要加锁
//////////////////////////////////////////////////////////////////////////////////////////////////
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//针对满二叉树的结构，查询时将根节点指针朝反向移动一个GRAIN
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
	DWORD Data[GRAIN_SIZE_IN_DWORD - 1];	//粒度8B
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

