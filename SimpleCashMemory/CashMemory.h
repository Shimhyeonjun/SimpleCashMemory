#ifndef CASHMEMORY_H

#ifndef _WIN64									// 컴파일 대상이 32비트

/*
	32비트 컴퓨터라고 가정하고 아래의 값을 결정
*/

#define CASHMEMORY_H

#define CASH_MAX_INDEX		64							// 최대 인덱스 개수
#define CASH_LINE_WAY		8							// 인덱스당 웨이 개수
#define CASH_LINE			64							// 캐시 라인 크기
#define TAG_LINE			4							// 태그 라인 크기

#define BIT_MASK_INDEX		0x00000fc0					// 인덱스 마스크
#define BIT_MASK_OFFSET		0x0000003f					// 오프셋 마스크
#define BIT_MASK_TAG		0xfffff000					// 태그 마스크

class CashMemory
{
private:
	static CashMemory* cashMemory;
	char* dataSpace;									// 데이터 저장 주소
	char* tagSpace;										// 태그 저장 주소
	char* useCount;										// 참조 카운터 저장 주소
	char* useflag;										// 사용 여부 저장 주소

	int hitCount;										// 캐시 히트 개수
	int missCount;										// 캐시 미스 개수	
	int changeCount;									// 교체 개수

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
