#include <iostream>
#include "CashMemory.h"

#define TESTCASE1           // 테스트 케이스 정의
#define MAX_SIZE 100

#define MAX_ROW 100
#define MAX_COL 10


int main(void)
{
    CashMemory* cashMemory = CashMemory::getCashMemoryInstance();

    /*
        1차원 배열 테스트
        
        결론
        적재시 인근 값들도 저장되기 때문에 캐시 히트가 높다.
    */

    #ifdef TESTCASE1

    int arr[MAX_ROW * MAX_COL];     
    int arr2[MAX_ROW * MAX_COL];    

    for (int i = 0; i < MAX_ROW * MAX_COL; i++)
    {
        arr[i] = i;
        arr2[i] = 0;
    }
   
    for (int i = 0; i < MAX_ROW * MAX_COL; i++)
    {
        std::cout << "Requrie data : " << arr[i] << std::endl;
        std::cout << "Requrie data address : " << (void*)&arr[i] << " [" << (unsigned int)&arr[i] << "] " << std::endl;
        cashMemory->getData((char*) & arr2[i], (const char*) & arr[i], sizeof(arr[i]));
        std::cout << "Get data : "<< arr2[i] << std::endl;

        // 캐시 메모리 뷰
        // cashMemory->showMemoryView();
        
    }
    
    // 캐시 히트율
    cashMemory->showCashHitRate();

    #endif
    
    
    /*
        2차원 배열 테스트
        
        결론
        2차원 배열도 1차원 배열과 같은 선형구조
        따라서 적재시 인근 값들도 저장되기 때문에 캐시 히트가 높다.
    */
    
    #ifdef TESTCASE2

    int arr3[MAX_ROW][MAX_COL];
    int arr4[MAX_ROW][MAX_COL];

    for (int i = 0; i < MAX_ROW; i++)
    {
        for (int j = 0; j < MAX_COL; j++)
        {
            arr3[i][j] = i * 10 + j;
            arr4[i][j] = 0;
        }
    }

    // 행 번호 먼저 증가
    for (int i = 0; i < MAX_ROW * 10; i++)
    {
        for (int j = 0; j < MAX_COL; j++)
        {
            std::cout << "Requrie data : " << arr3[j][i] << std::endl;
            std::cout << "Requrie data address : " << (void*)&arr3[j][i] << " [" << (unsigned int)&arr3[j][i] << "] " << std::endl;
            cashMemory->getData((char*)&arr4[j][i], (const char*)&arr3[j][i], sizeof(arr3[j][i]));
            std::cout << "Get data : " << arr3[j][i] << std::endl;

            // 캐시 메모리 뷰
            // cashMemory->showMemoryView();
        }
    }    

    // 캐시 히트율
    cashMemory->showCashHitRate();
    
    #endif

    /*
        동적할당 테스트
        
        결론
        배열에 비해 연속된 메모리 공간을 할당하지 않아서 캐시 미스가 높다.
        캐시 미스도 높지만, 교체 카운트도 높게 나옴
    */
    #ifdef TESTCASE3

    int* arr5[MAX_ROW * MAX_COL];
    int* arr6[MAX_ROW * MAX_COL];

    for (int i = 0; i < MAX_ROW * MAX_COL; i++)
    {
        arr5[i] = (int*)malloc(sizeof(int));
        *(arr5[i]) = i;

        arr6[i] = (int*)malloc(sizeof(int));
        *(arr6[i]) = 0;
    }

    for (int i = 0; i < MAX_ROW * MAX_COL; i++)
    {
        std::cout << "Requrie data : " << *(arr5[i]) << std::endl;
        std::cout << "Requrie data address : " << (void*)arr5[i] << " [" << (unsigned int)arr5[i] << "] " << std::endl;
        cashMemory->getData((char*)arr6[i], (const char*)arr5[i], sizeof( *(arr5[i])));
        std::cout << "Get data : " << *(arr6[i]) << std::endl;
        
        // 캐시 메모리 뷰
        // cashMemory->showMemoryView();

    }

    // 캐시 히트율
    cashMemory->showCashHitRate();
    
    #endif

    return 0;
}