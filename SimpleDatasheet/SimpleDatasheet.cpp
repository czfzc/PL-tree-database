// SimpleDatasheet.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "DataStruct.h"

/*

*
|  \
*    *
|\   |\
* *   * *
|\ |\ 






*/

int main()
{
	TRecordTree tre;
	tre.lpRaw = new TNode[4096];
	auto temp = tre.lpRoot();

	SETHASRIGHTCHILD((temp + 1)->Head);

	SETHOLDLEFTCHILD((temp + 3)->Head);

	(temp + 3)->Head |= 1;
	(temp + 6)->Head |= 1;

	memset((temp + 3)->Data, 0xff, sizeof(TNode::Data));
	memset((temp + 6)->Data, 0xff, sizeof(TNode::Data));
	TPtrRecord newptr;
	newptr.Mark = 1;
	newptr.Tree = &tre;
	
	CompletePtr(&newptr);

	auto k = *newptr;

	list<int>_temp;
	DWORD _k;
	(temp + 8)->Head |= 1;
	SubAllocate(&tre, 50, &_k, &_temp);

	_asm int 3
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
