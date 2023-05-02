#define _SOCKETAPI_
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <list>
#include <synchapi.h>
#include <time.h>
#include <conio.h>

#define TOTAL_THREAD_COUNT	6
#define VK_Q	0x51
#define VK_S	0x53
#define WAIT_EXIT_EVENT	WAIT_OBJECT_0
#define WAIT_SAVE_EVENT	WAIT_OBJECT_0 + 1

std::list<int> gList;
SRWLOCK lock = SRWLOCK_INIT;

HANDLE hEventExitThread;
HANDLE hEventSaveThread;

HANDLE hPrintThread;
HANDLE hDeleteThread;
HANDLE hWorkerThread1;
HANDLE hWorkerThread2;
HANDLE hWorkerThread3;
HANDLE hSaveThread;

HANDLE hThreadArr[TOTAL_THREAD_COUNT];
HANDLE hEventSaveThreadArr[2];

bool Init(void);
void CleanUp(void);
void MainThread(void);

unsigned _stdcall PrintThread(void* args);
unsigned _stdcall DeleteThread(void* args);
unsigned _stdcall WorkerThread(void* args);
unsigned _stdcall SaveThread(void* args);

int main(void)
{
	if (!Init())
	{
		CleanUp();
		return -1;
	}

	MainThread();
	CleanUp();
	return 0;
}

bool Init(void)
{
	hEventExitThread = CreateEvent(nullptr, true, false, L"EventExitThread");
	if (hEventExitThread == nullptr)
	{
		printf("Exit �̺�Ʈ��ü ���� ����: %d", GetLastError());
		return false;
	}

	hEventSaveThread = CreateEvent(nullptr, false, false, L"EventSaveThread");
	if (hEventSaveThread == nullptr)
	{
		printf("Save �̺�Ʈ��ü ���� ����: %d\n", GetLastError());
		return false;
	}

	hEventSaveThreadArr[0] = hEventExitThread;
	hEventSaveThreadArr[1] = hEventSaveThread;

	hPrintThread = (HANDLE)_beginthreadex(nullptr, 0, PrintThread, nullptr, 0, nullptr);
	if (hPrintThread == nullptr)
	{
		SetEvent(hEventExitThread);
		printf("PrintThread ���� ����: %d\n", GetLastError());
		return false;
	}

	hDeleteThread = (HANDLE)_beginthreadex(nullptr, 0, DeleteThread, nullptr, 0, nullptr);
	if (hDeleteThread == nullptr)
	{
		SetEvent(hEventExitThread);
		printf("DeleteThread ���� ����: %d\n", GetLastError());
		return false;
	}

	hWorkerThread1 = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, (void*)1, 0, nullptr);
	if (hWorkerThread1 == nullptr)
	{
		SetEvent(hEventExitThread);
		printf("WorkerThread1 ���� ����: %d\n", GetLastError());
		return false;
	}

	hWorkerThread2 = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, (void*)2, 0, nullptr);
	if (hWorkerThread1 == nullptr)
	{
		SetEvent(hEventExitThread);
		printf("WorkerThread2 ���� ����: %d\n", GetLastError());
		return false;
	}

	hWorkerThread3 = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, (void*)3, 0, nullptr);
	if (hWorkerThread1 == nullptr)
	{
		SetEvent(hEventExitThread);
		printf("WorkerThread3 ���� ����: %d\n", GetLastError());
		return false;
	}

	hSaveThread = (HANDLE)_beginthreadex(nullptr, 0, SaveThread, nullptr, 0, nullptr);
	if (hSaveThread == nullptr)
	{
		SetEvent(hEventExitThread);
		printf("SaveThread ���� ����: %d\n", GetLastError());
		return false;
	}

	hThreadArr[0] = hPrintThread;
	hThreadArr[1] = hDeleteThread;
	hThreadArr[2] = hWorkerThread1;
	hThreadArr[3] = hWorkerThread2;
	hThreadArr[4] = hWorkerThread3;
	hThreadArr[5] = hSaveThread;

	return true;
}

void CleanUp(void)
{
	CloseHandle(hEventExitThread);
	CloseHandle(hEventSaveThread);

	CloseHandle(hPrintThread);
	CloseHandle(hDeleteThread);
	CloseHandle(hWorkerThread1);
	CloseHandle(hWorkerThread2);
	CloseHandle(hWorkerThread3);
	CloseHandle(hSaveThread);
}

void MainThread(void)
{
	DWORD result;
	for (;;)
	{
		if (_kbhit())
		{
			_getch();

			if (GetAsyncKeyState(VK_S))
			{
				SetEvent(hEventSaveThread);
			}
			else if (GetAsyncKeyState(VK_Q))
			{
				printf("������ ���� ����\n");
				SetEvent(hEventExitThread);
				result = WaitForMultipleObjects(TOTAL_THREAD_COUNT, hThreadArr, true, INFINITE);
				if (result == WAIT_FAILED)
				{
					printf("Main ������ �ٸ� ������ ���� �����߻�: %d\n", GetLastError());
					break;
				}
				printf("������ ���� �Ϸ�\n");
				break;
			}
		}
	}
}



unsigned _stdcall PrintThread(void* args)
{
	DWORD result;
	for (;;)
	{
		result = WaitForSingleObject(hEventExitThread, 1000);
		switch (result)
		{
		case WAIT_OBJECT_0:
			printf("Print ������ ����\n");
			return 0;
		case WAIT_TIMEOUT:
		{
			AcquireSRWLockShared(&lock);
			std::list<int>::iterator iter = gList.begin();
			printf("���: ");
			for (; iter != gList.end(); ++iter)
			{
				printf("%d ", *iter);
			}
			printf("\n");
			ReleaseSRWLockShared(&lock);
			continue;
		}
		case WAIT_FAILED:
			printf("Print ������ ���� �̺�Ʈ �����߻�: %d\n", GetLastError());
		}
		break;
	}
	return -1;
}

unsigned _stdcall DeleteThread(void* args)
{
	DWORD result;
	for (;;)
	{
		result = WaitForSingleObject(hEventExitThread, 333);
		switch (result)
		{
		case WAIT_OBJECT_0:
			printf("Delete ������ ����\n");
			return 0;
		case WAIT_TIMEOUT:
			AcquireSRWLockExclusive(&lock);
			if (gList.size() > 0)
			{
				gList.pop_back();
			}
			ReleaseSRWLockExclusive(&lock);
			continue;
		case WAIT_FAILED:
			printf("Delete ������ ���� �̺�Ʈ �����߻�: %d\n", GetLastError());
		}
		break;
	}
	return -1;
}

unsigned _stdcall WorkerThread(void* args)
{
	unsigned int threadIndex = (unsigned int)args;
	srand(_time32(nullptr) + threadIndex);
	DWORD result;
	for (;;)
	{
		result = WaitForSingleObject(hEventExitThread, 1000);
		switch (result)
		{
		case WAIT_OBJECT_0:
			printf("Worker ������%d ����\n", threadIndex);
			return 0;
		case WAIT_TIMEOUT:
			AcquireSRWLockExclusive(&lock);
			gList.push_back(rand() % MAXINT32);
			ReleaseSRWLockExclusive(&lock);
			continue;
		case WAIT_FAILED:
			printf("Worker ������%d ���� �̺�Ʈ �����߻�: %d\n", threadIndex, GetLastError());
		}
		break;
	}
	return -1;
}

unsigned _stdcall SaveThread(void* args)
{
	DWORD result;
	FILE* pFile;
	for (;;)
	{
		result = WaitForMultipleObjects(2, hEventSaveThreadArr, false, INFINITE);
		switch (result)
		{
		case WAIT_EXIT_EVENT:
			printf("Save ������ ����\n");
			return 0;
		case WAIT_SAVE_EVENT:
		{
			printf("���� �����û\n");
			int fileSize = 0;
			char* buffer = nullptr;
			AcquireSRWLockShared(&lock);
			if (fopen_s(&pFile, "integer_list.txt", "rb") == 0)
			{
				fseek(pFile, 0, SEEK_END);
				fileSize = ftell(pFile);
				fseek(pFile, 0, SEEK_SET);
				if (fileSize != 0)
				{
					buffer = new char[fileSize];
					fread(buffer, fileSize, 1, pFile);
				}

				fclose(pFile);
			}

			errno_t result;
			if (result = fopen_s(&pFile, "integer_list.txt", "wb"))
			{
				printf("���� ���� ���� wr ��� �����ڵ�: %d\n", result);
				continue;
			}

			if (buffer != nullptr)
			{
				fwrite(buffer, fileSize, 1, pFile);
			}
			
			std::list<int>::iterator iter = gList.begin();
			for (; iter != gList.end(); ++iter)
			{
				fprintf_s(pFile, "%d ", *iter);
			}
			fprintf_s(pFile, "\n");
			fclose(pFile);
			ReleaseSRWLockShared(&lock);
			continue;
		}
		case WAIT_FAILED:
			printf("Save ������ �����߻�: %d\n", GetLastError());
		}
		break;
	}

	return -1;
}