#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <string.h>
#define BIG_SIZE 1073741824 //1 GB

typedef struct result {
    int numberOfLargeFiles;
    FILETIME timeCreated;
} result;

int search(LPCWSTR path, result* res) {
    HANDLE handle;
    WIN32_FIND_DATAW find;
    int isEmpty = 1;

    if (!SetCurrentDirectoryW(path)) {
        return 1;
    }
    else {
        handle = FindFirstFileW(L"*", &find);

        if (handle == INVALID_HANDLE_VALUE) {
            printf("ERROR!\n");
            return 1;
        } else {
            do {
                if ((wcscmp(find.cFileName, L".") != 0) && (wcscmp(find.cFileName, L"..") != 0)) {
                    isEmpty = 0;
                    if (find.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
                        search(find.cFileName, res);
                        SetCurrentDirectoryW(L"..");
                    } else {
                        if (find.nFileSizeLow > BIG_SIZE) {
                            res->numberOfLargeFiles++;
                        }
                        if ((find.ftCreationTime.dwHighDateTime < res->timeCreated.dwHighDateTime) || (find.ftCreationTime.dwLowDateTime < res->timeCreated.dwLowDateTime)) {
                            res->timeCreated.dwHighDateTime = find.ftCreationTime.dwHighDateTime;
                            res->timeCreated.dwLowDateTime = find.ftCreationTime.dwLowDateTime;
                        }
                    }
                }
            } while (FindNextFileW(handle, &find));
            FindClose(handle);
            return isEmpty;

        }
    }
}


int main(int argc, char* argv[]) {
    int j;
    SYSTEMTIME t;
    result res;

    res.numberOfLargeFiles = 0;
    res.timeCreated.dwHighDateTime = 0xffffffff;
    res.timeCreated.dwLowDateTime = 0xffffffff;

    if (argc < 2) {
        printf("Directory path is not provided.\n");
        return 5;
    }

    wchar_t unicodePath[MAX_PATH];
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, unicodePath, MAX_PATH, argv[1], _TRUNCATE);

    DWORD dwAttrib = GetFileAttributesW(unicodePath);
    if (dwAttrib == INVALID_FILE_ATTRIBUTES || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
        printf("Directory doesn't exist.\n");
        return 1;
    }

    j = search(unicodePath, &res);

    if (j == 1) {
        printf("The specified directory is empty!\n");
        return 2;
    }

    printf("Number of large files: %d\n", res.numberOfLargeFiles);
    FileTimeToSystemTime(&(res.timeCreated), &t);
    printf("Time of creation of the oldest file: %02d.%02d.%d.\n", t.wDay, t.wMonth, t.wYear);
    return 0;
}

