#pragma once
#include <windows.h>
#include <ppl.h>

#include <queue>
#include <list>
#include <string>
#include <array>

using namespace std;
using namespace concurrency;

//ÿ�������һ����
enum enDataType {
	T_DWORD,
	T_DOUBLE,
	T_STRING
};

struct stAttribute {
	int Columns;
	list<enDataType> ColumnType;
};

//��ķ�����Ϊ���㣺
//�ײ㣺����飬ͳ�ƿ��Ƶ�ʣ���̬������Щ�飻ʵ�ּ�¼�Ľṹ��ǣ�ʵ�ֲ�ѯ����飬�༭�����ӣ�ɾ�����༭����Լ��Ϊɾ�������ӣ����Ĺ���
constexpr auto SINGLE_BLOCK_SIZE = 4096;
constexpr auto ONLINE_BLOCKS_COUNT = 64;


template <typename T>
struct stBlock {
	string HeadFilePath;
	char* Raw;
};
//�ϲ㣺�ṩ�첽/ͬ���ĵ���ѯ�����ѯ��

class MyDataSheet
{
public:
private:
	string HeadFilePath;

};

