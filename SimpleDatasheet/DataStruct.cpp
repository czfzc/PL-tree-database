#include "DataStruct.h"

constexpr size_t __LEFT = 0;
constexpr size_t __RIGHT = 1;
constexpr size_t __ROOT = 1;

constexpr size_t LEFT_CHILD(const size_t& _Index) { return (__LEFT + (_Index << 1)); };
constexpr size_t RIGHT_CHILD(const size_t& _Index) { return (__RIGHT + (_Index << 1)); };
constexpr size_t PARENT(const size_t& _Index) { return _Index >> 1; };

constexpr DWORD INVALID_MARK = 0;

void inline __inner_convert_index_to_trail(DWORD index, DWORD* _trail)
{
	*_trail = 0;
	while (index > 1)
	{
		*_trail |= index & 1;
		*_trail <<= 1;
		index >>= 1;
	}
	*_trail >>= 1;
}

void __inner_get_record_index(TNode* lpRoot, list<int>* _List, size_t i)//inner
{
	_List->push_back(i);
	if (IFHOLDLEFTCHILD((lpRoot + i)->Head))
		__inner_get_record_index(lpRoot, _List, LEFT_CHILD(i));	
	if (IFHOLDRIGHTCHILD((lpRoot + i)->Head))
		__inner_get_record_index(lpRoot, _List, RIGHT_CHILD(i));
}

void __inner_get_empty_index(TNode* lpRoot, list<int>* _List, DWORD Index)//unsafe
{
	if (GETMARK((lpRoot + Index)->Head))
		return;
	_List->push_back(Index);
	__inner_get_empty_index(lpRoot, _List, LEFT_CHILD(Index));
	__inner_get_empty_index(lpRoot, _List, RIGHT_CHILD(Index));
}

void __inner2_get_empty_index(const int& MaxNodeCount, TNode* lpRoot, list<int>* _List, DWORD Index, bool* Flags)
{
	if (Index > MaxNodeCount)
		return;
	if (GETMARK((lpRoot + Index)->Head))
		return;
	_List->push_back(Index);
	*(Flags + Index) = 1;
	__inner2_get_empty_index(MaxNodeCount, lpRoot, _List, LEFT_CHILD(Index), Flags);
	__inner2_get_empty_index(MaxNodeCount, lpRoot, _List, RIGHT_CHILD(Index), Flags);
}

bool __inner_allocate_index(const int& MaxNodeCount, TNode* lpRoot, list<int>* _List, DWORD _Size)//寻找大的连通子图
{
	bool* Flags = new bool[MaxNodeCount + 1];
	for (int i = 1; i <= MaxNodeCount; i++)
		Flags[i] = 0;
	for (int i = 1; i <= MaxNodeCount; i++)
	{
		if (!Flags[i])
		{
			_List->clear();
			__inner2_get_empty_index(MaxNodeCount, lpRoot, _List, i, Flags);
			//注意二叉树的连通性是传递的：如果AB两个Node分离，那么他们的任意子结点分离
			if (_List->size() >= _Size)
				return 1;
		}
	}
	delete Flags;
	return 0;
}

bool __inner_allocate_index(const int& MaxNodeCount, TNode* lpRoot, DWORD i, list<int>* _List, bool& flag, 	
	DWORD& temproot, DWORD size, DWORD& tempsize)//error!!
{
	if (i > MaxNodeCount)
		return 0;//zhongxubianli
	if (flag)
	{
		if (GETMARK((lpRoot + i)->Head))
		{
			if (tempsize >= size)
				return 1;
			else
			{
				flag = 0;
				tempsize = 0;
				_List->clear();
			}
		}
		else {
			_List->push_back(i);
			tempsize++;
			if (tempsize >= size)
				return 1;
		}
	}
	else {
		if (!GETMARK((lpRoot + i)->Head))
		{
			tempsize = 1;
			flag = 1;
			temproot = i;
			_List->clear();
			_List->push_back(i);
		}
	}
	if (__inner_allocate_index(MaxNodeCount, lpRoot, LEFT_CHILD(i), _List, flag, temproot, size, tempsize))
		return 1;
	if (__inner_allocate_index(MaxNodeCount, lpRoot, RIGHT_CHILD(i), _List, flag, temproot, size, tempsize))
		return 1;
	return 0;
}


TRecord& TPtrRecord::operator*()
{
	TRecord* temp = new TRecord;
	TNode* lpNode = this->Tree->lpRoot();
	size_t i = __ROOT;
	DWORD trail = *this->Trail;
	while (i <= this->Tree->MaxNodeCount)
	{
		if ((lpNode + i)->Mark == this->Mark)
		{
			list<int>List;
			__inner_get_record_index(lpNode, &List, i);
			temp->Head = (lpNode + i)->Head;
			temp->SizeInNodes = List.size();
			temp->lpRaw = new DWORD[GRAIN_SIZE_IN_DWORD * temp->SizeInNodes];
			DWORD* ptr = (DWORD*)temp->lpRaw;
			for (auto& p : List)
			{
				memcpy(ptr, (lpNode + p)->Data, sizeof(DWORD) * (GRAIN_SIZE_IN_DWORD - 1));
				ptr += GRAIN_SIZE_IN_DWORD - 1;
			}
			return *temp;
		}
		if (!IFHASLEFTCHILD((lpNode + i)->Head))
			if (!IFHASRIGHTCHILD((lpNode + i)->Head))
			{
				temp->Head = INVALID_MARK;
				return *temp;
			}
		i <<= 1;
		i |= trail & 1;
		trail >>= 1;
	}
	temp->Head = INVALID_MARK;
	return *temp;
}

TRecord* TPtrRecord::operator->()
{
	return &(this->operator*());
}

int GetTrailByMark(TRecordTree* _Tree, DWORD _Mark, TTrail* _Trail)
{
	if (_Tree == nullptr)
		return -1;
	bool flag = 0;
	_Mark = GETMARK(_Mark);
	size_t i = __ROOT;
	TNode* lpNode = _Tree->lpRoot();
	for (; i <= _Tree->MaxNodeCount; i++)
		if ((lpNode + i)->Mark == _Mark)
		{
			flag = 1;
			break;
		}
	if (flag)
	{
		__inner_convert_index_to_trail(i, _Trail);
		return 0;
	}
	else
		return -1;
}

int CompletePtr(TPtrRecord* _UncompletedPtr)
{
	if (_UncompletedPtr == nullptr)
		return -1;
	if (_UncompletedPtr->Trail != nullptr)
		return 0;
	_UncompletedPtr->Trail = new DWORD;
	return GetTrailByMark(_UncompletedPtr->Tree, _UncompletedPtr->Mark, _UncompletedPtr->Trail);
}

int SubAllocate(TRecordTree* _Tree, DWORD _Size, DWORD* _Index, list<int>* _List)
{
	if (__inner_allocate_index(_Tree->MaxNodeCount, _Tree->lpRoot(), _List, _Size))
		return 0;
	return -1;
}

int InsertRecord(TRecord* _Record, DWORD _Index, TPtrRecord* _Ptr)
{
	list<int> temp;
	temp.clear();
	//首先设置沿途标记

	return 0;
}
