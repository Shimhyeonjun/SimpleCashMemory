#ifndef CASHMEMORY_H

#ifndef _WIN64									// ������ ����� 32��Ʈ

/*
	32��Ʈ ��ǻ�Ͷ�� �����ϰ� �Ʒ��� ���� ����
*/

#define CASHMEMORY_H

#define CASH_MAX_INDEX		64							// �ִ� �ε��� ����
#define CASH_LINE_WAY		8							// �ε����� ���� ����
#define CASH_LINE			64							// ĳ�� ���� ũ��
#define TAG_LINE			4							// �±� ���� ũ��

#define BIT_MASK_INDEX		0x00000fc0					// �ε��� ����ũ
#define BIT_MASK_OFFSET		0x0000003f					// ������ ����ũ
#define BIT_MASK_TAG		0xfffff000					// �±� ����ũ

class CashMemory
{
private:
	static CashMemory* cashMemory;
	char* dataSpace;									// ������ ���� �ּ�
	char* tagSpace;										// �±� ���� �ּ�
	char* useCount;										// ���� ī���� ���� �ּ�
	char* useflag;										// ��� ���� ���� �ּ�

	int hitCount;										// ĳ�� ��Ʈ ����
	int missCount;										// ĳ�� �̽� ����	
	int changeCount;									// ��ü ����

private:
	CashMemory();

	bool allocate(const char* src);

public:
	static CashMemory* getCashMemoryInstance();

	~CashMemory();

	bool getData(char * des, const char* src, size_t srcSize);
	void showMemoryView();
	void showCashHitRate();
};
#endif

#endif 
