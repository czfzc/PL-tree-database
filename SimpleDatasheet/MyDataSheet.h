#pragma once
#include <windows.h>
#include <ppl.h>

#include <queue>
#include <list>
#include <string>
#include <array>

using namespace std;
using namespace concurrency;

//每个类管理一个表
enum enDataType {
	T_DWORD,
	T_DOUBLE,
	T_STRING
};

struct stAttribute {
	int Columns;
	list<enDataType> ColumnType;
};

//表的方法分为两层：
//底层：管理块，统计块的频率，动态加载这些块；实现记录的结构标记，实现查询，检查，编辑，增加，删除（编辑不能约化为删除和增加！）的功能
constexpr auto SINGLE_BLOCK_SIZE = 4096;
constexpr auto ONLINE_BLOCKS_COUNT = 64;


template <typename T>
struct stBlock {
	string HeadFilePath;
	char* Raw;
};
//上层：提供异步/同步的单查询，多查询，

class MyDataSheet
{
public:
private:
	string HeadFilePath;

};

