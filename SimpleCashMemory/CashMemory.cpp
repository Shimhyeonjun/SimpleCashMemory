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

	// src가 포함된 64바이트 적재 공간의 시작주소
	const char* startAddress = (char*)((unsigned int)src & 0xffffffc0);
	
	// 저장할 위치
	// 우선 순위는 빈 공간, 가장 접근이 적은 공간
	
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
	
	// 태그, 데이터를 저장할 시작 주소
	tagAddress	= tagAddress + (TAG_LINE * wayIndex);
	dataAddress = dataAddress + (CASH_LINE * wayIndex);

	// 교체 공간 확인용
	if (useflag[wayIndex + (index * CASH_LINE_WAY)] != 0)
		changeCount++;

	// 사용 여부, 참조 카운터 세팅
	useflag[wayIndex + (index  * CASH_LINE_WAY)] = 1;
	useCount[wayIndex + (index * CASH_LINE_WAY)] = 0;

	// 태그 값 저장	
	*(int*)tagAddress = tag;
		
	// 64bytes를 한번에 적재한다.
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
		// 태그, 인덱스, 오프셋
		unsigned int tag	= ((unsigned int)nextSrc & BIT_MASK_TAG) >> 12;
		unsigned int index	= ((unsigned int)nextSrc & BIT_MASK_INDEX) >> 6;
	 	unsigned int offset	= (unsigned int)nextSrc & BIT_MASK_OFFSET;

		// 해당 인덱스의 데이터 저장 시작주소, 태그 저장 시작주소
		const char* tagAddress	= tagSpace + (TAG_LINE * CASH_LINE_WAY * index);
		const char* dataAddress = dataSpace + (CASH_LINE * CASH_LINE_WAY * index);
		
		bool isfind	 = false;
		int wayIndex = 0;

		// 인덱스 별로 총 8개의 캐쉬 라인
		for (int i = 0; i < CASH_LINE_WAY; i++)
		{
			// 태그 비교
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

		// 일치하는 태그가 있다면, 캐시 히트
		if (isfind)
		{
			for (size_t j = 0; j < srcSize; j++)
			{
				if (j + offset >= CASH_LINE)
					break;

				des[desSize++] = dataAddress[j + offset];
			}

			// 해당 캐시라인 참조 카운트 증가
			useCount[wayIndex + (index * CASH_LINE_WAY)]++;
		}

		srcSize = srcSize - desSize;

		// 전부 얻었으면 종료
		if (srcSize <= 0)
		{			
			// 미스 -> 적재 -> 히트의 한 사이클은 히트율 계산을 위해서 제외
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
			
			// 부족하거나, 없을 경우에는 할당
			if (nextSrc == src + desSize)
			{
				// 할당에서 에러면 더이상 진행 불가
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