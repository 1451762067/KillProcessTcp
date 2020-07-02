// cxx.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

//KillTcpConnect有关头文件
#include <ws2tcpip.h>  
#include <iphlpapi.h>

//和获取进程列表有关的头文件
#include <tlhelp32.h>

//本demo用到的容器
#include <vector>
#include <map>


#include "stdio.h"
#include <stdlib.h>
#include <iostream>


//和KillTcpConnect有关的静态库
#pragma comment(lib, "iphlpapi.lib") 
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Wininet.lib")

using namespace std;

int GetProcessList(multimap<string, DWORD>  &ProcessList)
{
    HANDLE hSnapshort = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshort == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    // 获得线程列表，里面记录了线程的详细信息，再使用Thread32First和Thread32Next遍历快照中记录的每个线程信息  
    PROCESSENTRY32 stcProcessInfo;
    stcProcessInfo.dwSize = sizeof(stcProcessInfo);

    BOOL  bRet = Process32First(hSnapshort, &stcProcessInfo);
    while (bRet)
    {
        
        ProcessList.insert(make_pair(stcProcessInfo.szExeFile, stcProcessInfo.th32ProcessID));
        bRet = Process32Next(hSnapshort, &stcProcessInfo);
    }
    CloseHandle(hSnapshort);
}

BOOL KillTcpConnect(int ProcID)  //关闭进程中的TCP连接
{
    PMIB_TCPTABLE_OWNER_PID pTcpTable;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
    int i;

    pTcpTable = (MIB_TCPTABLE_OWNER_PID*)malloc(sizeof(MIB_TCPTABLE_OWNER_PID));
    if (pTcpTable == NULL)
    {
        return FALSE;
    }

    dwSize = sizeof(MIB_TCPTABLE_OWNER_PID);

    if ((dwRetVal = GetExtendedTcpTable(pTcpTable, &dwSize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0)) == ERROR_INSUFFICIENT_BUFFER)
    {
        free(pTcpTable);
        pTcpTable = (MIB_TCPTABLE_OWNER_PID*)malloc(dwSize);
        if (pTcpTable == NULL)
        {
            //OutputDebugStringA("Error allocating memory\n");
            return FALSE;
        }
    }

    if ((dwRetVal = GetExtendedTcpTable(pTcpTable, &dwSize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0)) == NO_ERROR)
    {
        MIB_TCPROW  TcpRow;    // Declare A TCP Raw 

        for (i = 0; i < (int)pTcpTable->dwNumEntries; i++)
        {
            if (ProcID == pTcpTable->table[i].dwOwningPid)
            {
                if (_stricmp("0.0.0.0", inet_ntoa(*(in_addr*)&pTcpTable->table[i].dwRemoteAddr)) != 0)
                {
                    char LocalPort[10] = { 0 };
                    char RemotePort[10] = { 0 };
                    _snprintf_s(LocalPort, sizeof(LocalPort) - 1, "%d", htons((u_short)pTcpTable->table[i].dwLocalPort));
                    _snprintf_s(RemotePort, sizeof(RemotePort) - 1, "%d", htons((u_short)pTcpTable->table[i].dwRemotePort));

                    TcpRow.dwLocalPort = htons(atoi(LocalPort));
                    TcpRow.dwRemotePort = htons(atoi(RemotePort));
                    TcpRow.dwLocalAddr = inet_addr(inet_ntoa(*(in_addr*)&pTcpTable->table[i].dwLocalAddr));
                    TcpRow.dwRemoteAddr = inet_addr(inet_ntoa(*(in_addr*)&pTcpTable->table[i].dwRemoteAddr));
                    TcpRow.dwState = MIB_TCP_STATE_DELETE_TCB;
                    if (SetTcpEntry(&TcpRow) == ERROR_SUCCESS)
                    {
                        //OutputDebugStringA("KILLCONNECT ERROR。。…。…。………。。…。……");
                    }
                }

            }

        }
    }
    else
    {
        free(pTcpTable);
        return FALSE;
    }

    if (pTcpTable != NULL)
    {
        free(pTcpTable);
        pTcpTable = NULL;
    }

    return TRUE;
}

bool ParseValue(const std::string& _s, vector<string>& v, char separator)
{
    std::string sPart;
    size_t pos;
    std::string s = _s;
    std::string sp;

    sp.push_back(separator);

    while (!s.empty())
    {
        pos = s.find(sp.c_str());

        if (pos == std::string::npos)
        {
            sPart = s;
            s.erase();
        }
        else
        {
            sPart = s.substr(0, pos);
            s.erase(0, pos + 1);
        }

        if (!sPart.empty())
        {
            v.push_back(sPart);
        }
    }

    return true;
}


int main(int argc, char** argv)
{
    multimap<string, DWORD> processList;
    vector<DWORD> targetPIDs;
    vector<string> exeList;

    do{
        char exe[20];
        printf("输入要杀死的进程名:");
        memset(exe, 0, sizeof(exe));
        scanf("%s", exe);

        //解析进程名列表
        exeList.clear();
        ParseValue(exe, exeList, ',');

        //获取最近进程列表
        processList.clear();
        GetProcessList(processList);

        //获取目标进程的进程ID列表
        targetPIDs.clear();
        for (auto p : processList)
        {
            for (auto e : exeList)
            {
                if (p.first == e)
                {
                    targetPIDs.push_back(p.second);
                    break;
                }
            }
        }
        //杀死
        for (auto pid : targetPIDs) {
            if (KillTcpConnect(pid))
                printf("%d 网络杀死成功\n", pid);
            else
                printf("%d 网络杀死失败\n", pid);
        }
    } while (true);
    
    return 0;
}
