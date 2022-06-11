#include "CashMemory.h"
#include <iostream>

CashMemory* CashMemory::cashMemory = nullptr;

CashMemory::CashMemory() 
{
	dataSpace	= new char[CASH_LINE * CASH_LINE_WAY * CASH_MAX_INDEX];
	tagSpace	= new char[TAG_LINE * CASH_LINE_WAY * CASH_MAX_INDEX];
	useflag		= new char[CASH_MAX_INDEX * CASH_LINE_WAY];
	useCount	= new char[CASH_MAX_INDEX * CASH_LINE_WAY];
	missCount	= 0;
	hitCount	= 0;
	changeCount = 0;

	for (int i = 0; i < CASH_LINE * CASH_LINE_WAY * CASH_MAX_INDEX; ++i)
	{
		dataSpace[i] = 0;	
	}

	for (int i = 0; i < TAG_LINE * CASH_LINE_WAY * CASH_MAX_INDEX; ++i)
	{
		tagSpace[i] = 0;
	}

	for (int i = 0; i < CASH_MAX_INDEX * CASH_LINE_WAY; ++i)
	{
		useflag[i]	= 0;
		useCount[i] = 0;
	}
}

CashMemory::~CashMemory()
{
	delete[] dataSpace;
	delete[] tagSpace;
	delete[] useflag;
	delete[] useCount;
}

CashMemory* CashMemory::getCashMemoryInstance()
{
	if (cashMemory == nullptr)
		cashMemory = new CashMemory();

	return cashMemory;
}


bool CashMemory::allocate(const char* src)
{			
	unsigned int tag	= ((unsigned int)src & BIT_MASK_TAG) >> 12;
	unsigned int index	= ((unsigned int)src & BIT_MASK_INDEX) >> 6;
	
	char* tagAddress	= tagSpace + (TAG_LINE * CASH_LINE_WAY * index);
	char* dataAddress	= dataSpace + (CASH_LINE * CASH_LINE_WAY * index);

	// src�� ���Ե� 64����Ʈ ���� ������ �����ּ�
	const char* startAddress = (char*)((unsigned int)src & 0xffffffc0);
	
	// ������ ��ġ
	// �켱 ������ �� ����, ���� ������ ���� ����
	
	int min			= 9999;
	int wayIndex	= -1;

	for (int i = 0; i < CASH_LINE_WAY; i++)
	{
		int priority = useflag[index * CASH_LINE_WAY + i] + useCount[index * CASH_LINE_WAY + i];

		if (min > priority)
		{
			min		 = priority;
			wayIndex =  i;
		}	
	}

	if (wayIndex == -1)
		return false;
	
	// �±�, �����͸� ������ ���� �ּ�
	tagAddress	= tagAddress + (TAG_LINE * wayIndex);
	dataAddress = dataAddress + (CASH_LINE * wayIndex);

	// ��ü ���� Ȯ�ο�
	if (useflag[wayIndex + (index * CASH_LINE_WAY)] != 0)
		changeCount++;

	// ��� ����, ���� ī���� ����
	useflag[wayIndex + (index  * CASH_LINE_WAY)] = 1;
	useCount[wayIndex + (index * CASH_LINE_WAY)] = 0;

	// �±� �� ����	
	*(int*)tagAddress = tag;
		
	// 64bytes�� �ѹ��� �����Ѵ�.
	for(int i = 0; i < CASH_LINE; i++)
		dataAddress[i] = startAddress[i];

	return true;	
}

bool CashMemory::getData(char* des, const char* src, size_t srcSize)
{
	bool missFlag = false;
	const char* nextSrc = src;

	while(true)
	{
		// �±�, �ε���, ������
		unsigned int tag	= ((unsigned int)nextSrc & BIT_MASK_TAG) >> 12;
		unsigned int index	= ((unsigned int)nextSrc & BIT_MASK_INDEX) >> 6;
	 	unsigned int offset	= (unsigned int)nextSrc & BIT_MASK_OFFSET;

		// �ش� �ε����� ������ ���� �����ּ�, �±� ���� �����ּ�
		const char* tagAddress	= tagSpace + (TAG_LINE * CASH_LINE_WAY * index);
		const char* dataAddress = dataSpace + (CASH_LINE * CASH_LINE_WAY * index);
		
		bool isfind	 = false;
		int wayIndex = 0;

		// �ε��� ���� �� 8���� ĳ�� ����
		for (int i = 0; i < CASH_LINE_WAY; i++)
		{
			// �±� ��
			if (tag == *((int*)tagAddress))
			{
				isfind	 = true;
				wayIndex =  i;
				break;
			}				

			tagAddress = tagAddress + TAG_LINE;
			dataAddress = dataAddress + CASH_LINE;
		}
		
		size_t desSize = 0;

		// ��ġ�ϴ� �±װ� �ִٸ�, ĳ�� ��Ʈ
		if (isfind)
		{
			for (size_t j = 0; j < srcSize; j++)
			{
				if (j + offset >= CASH_LINE)
					break;

				des[desSize++] = dataAddress[j + offset];
			}

			// �ش� ĳ�ö��� ���� ī��Ʈ ����
			useCount[wayIndex + (index * CASH_LINE_WAY)]++;
		}

		srcSize = srcSize - desSize;

		// ���� ������� ����
		if (srcSize <= 0)
		{			
			// �̽� -> ���� -> ��Ʈ�� �� ����Ŭ�� ��Ʈ�� ����� ���ؼ� ����
			if (!missFlag)
			{
				hitCount++;
				std::cout << "Cash hit" << std::endl;
			}

			break;			
		}
		else
		{
			missFlag = true;
			missCount++;

			std::cout << "Cash miss" << std::endl;
			
			// �����ϰų�, ���� ��쿡�� �Ҵ�
			if (nextSrc == src + desSize)
			{
				// �Ҵ翡�� ������ ���̻� ���� �Ұ�
				if (!allocate(src))
					return false;
			}
				
			else
				nextSrc = src + desSize;
		}
	}

	return true;
}

void CashMemory::showMemoryView()
{
	std::cout << "\n-------------------------\n" << std::endl;
	std::cout << "Using cash line" << std::endl;
	std::cout << "Data cash start address : " << (void*)dataSpace << " [" << (unsigned int)dataSpace << "] " << std::endl << std::endl;

	for (int i = 0 ; i < CASH_MAX_INDEX * CASH_LINE_WAY; i++)
	{
		if (useflag[i] != 0)
		{
			char* AddressOfCashIndex = dataSpace + (CASH_LINE * (i / CASH_LINE_WAY));
			char* AddressOfWayIndex = dataSpace + (CASH_LINE * i);
			
			std::cout << "Cash index : " << i / CASH_LINE_WAY << std::endl;
			std::cout << "Way index : " << i % CASH_LINE_WAY << std::endl;
			std::cout << "Address of cash Index : " << (void*)AddressOfCashIndex << " [" << (unsigned int)AddressOfCashIndex << "] " << std::endl;
			std::cout << "Address of way Index : " << (void*)AddressOfWayIndex << " [" << (unsigned int)AddressOfWayIndex << "] " << std::endl;
			std::cout << "Number of uses : " << (unsigned int)useCount[i] << std::endl << std::endl;
		}
	}
	std::cout << "\n-------------------------\n" << std::endl;
}

void CashMemory::showCashHitRate()
{
	std::cout << "\n-------------------------\n" << std::endl;
	std::cout << "Total Count : " << hitCount + missCount << std::endl;
	std::cout << "Hit count : " << hitCount << std::endl;
	std::cout << "Miss count : " << missCount << std::endl;
	std::cout << "Change count : " << changeCount << std::endl;
	std::cout << "Hit rate : " << (double)(hitCount * 1000) / (hitCount + missCount)  / 10<< std::endl;
	std::cout << "Miss rate : " << (double)(missCount * 1000) / (hitCount + missCount) / 10<< std::endl;
	std::cout << "\n-------------------------\n" << std::endl;
}