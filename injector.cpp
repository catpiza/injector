/*
** Сурс код самого инъектора принадлежит какому-то ноунейму с форумов mpgh.net.
** Сурс чуть-чуть доработан для более удобного использования.
** Все права не защищены.
*/


#include <windows.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#include <conio.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN // ADVANCED WINAPI
#define CREATE_THREAD_ACCESS (PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ)

BOOL Inject(DWORD pID, const char * DLL_NAME);
DWORD GetTargetThreadIDFromProcName(const char * ProcName);

int main(int argc, char * argv[])
{
  DWORD pID = GetTargetThreadIDFromProcName("hl2.exe"); // Получаем ID процесса

  puts("Name of dll: ");

  char path[MAX_PATH] = {0};
  char name_of_dll[64];
  char adddll[4];

  strcpy (adddll, ".dll");
  scanf("%s",&name_of_dll);

  ////////////////////////////////////////////
  /*
  ** Если ты не указал в конце названия файла,
  ** что он формата '.dll' -
  ** то этот код его добавит автоматически
  */
  ////////////////////////////////////////////

  char *str = strstr (name_of_dll, ".dll"); // Ищем первое вхождение подстроки .dll в названии
  if (str == NULL)
  {
       strncat(name_of_dll, adddll, 4); // Добавление к имени длл ".dll", потому что мне лень вводить формат
  }
  else
  {
      // do nothing
  }


  GetFullPathName(name_of_dll, MAX_PATH, path, NULL); // Нахождение патча, где находится инъектор

  puts("Dll name:");
  puts(name_of_dll);

  ////////////////////////////////////////////
  /*
  ** Здесь идет проверка процесса,
  ** запущен ли он?
  ** Если он запущен - проводим инъекцию,
  ** в противном случае ждем
  */
  ////////////////////////////////////////////

  DWORD *ptr_pID = &pID;
  if (*ptr_pID == NULL)
  {
    puts("\nWaiting for the game");
    do
        {
            DWORD pID = GetTargetThreadIDFromProcName("hl2.exe");
            putchar('.');
            Sleep(1000);
        }
    while(*ptr_pID == 0);
  }

  // Инъекция

    if(!Inject(pID, path))
    {
        puts("DLL Not Loaded!");
    }
    else
    {
        puts("DLL Loaded!");
    }

    _getch();
  return 0;
}

BOOL Inject(DWORD pID, const char * DLL_NAME)
{
  HANDLE Proc;
  HMODULE hLib;
  char buf[50] = {0};
  LPVOID RemoteString, LoadLibraryEx; // Указатель (typedef void far *LPVOID;)

  if(!pID)
      return false;

  Proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
  if(!Proc)
  {
      return false;
  }

  LoadLibraryEx = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

  // Выделение места в процессе для нашей dll
  RemoteString = (LPVOID)VirtualAllocEx(Proc, NULL, strlen(DLL_NAME), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

  // Запись имени строки нашей дллки в выделенной заранее памяти
  WriteProcessMemory(Proc, (LPVOID)RemoteString, DLL_NAME, strlen(DLL_NAME), NULL);

  // Загрузка длд
  CreateRemoteThread(Proc, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryEx, (LPVOID)RemoteString, NULL, NULL);

  CloseHandle(Proc);
  return true;
}

DWORD GetTargetThreadIDFromProcName(const char * ProcName)
{
  PROCESSENTRY32 pe;
  HANDLE thSnapShot;
  BOOL retval, ProcFound = false;

  thSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if(thSnapShot == INVALID_HANDLE_VALUE)
  {
      return false;
  }

  pe.dwSize = sizeof(PROCESSENTRY32);

  retval = Process32First(thSnapShot, &pe);
  while(retval)
  {
      if(StrStrI(pe.szExeFile, ProcName))
      {
        return pe.th32ProcessID;
      }
      retval = Process32Next(thSnapShot, &pe);
  }
  return 0;
}
