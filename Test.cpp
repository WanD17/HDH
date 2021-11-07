#include <windows.h>
#include<iostream>
#include <stdio.h>
#include<vector>
#include<string>
#include<math.h>
using namespace std;

int hex2dec(const char num[]) {
    int len = strlen(num);
    int base = 1;
    int temp = 0;
    for (int i = len - 1; i >= 0; i--) {
        if (num[i] >= '0' && num[i] <= '9') {
            temp += (num[i] - 48) * base;
            base = base * 16;
        }
        else if (num[i] >= 'A' && num[i] <= 'F') {
            temp += (num[i] - 55) * base;
            base = base * 16;
        }
        else if (num[i] >= 'a' && num[i] <= 'f') {
            temp += (num[i] - 87) * base;
            base = base * 16;
        }
    }
    return temp;
}
void ReadBytes2Str(BYTE sector[], const char row[], const char col[], int num) {
    int col_dec = hex2dec(col);
    int row_dec = hex2dec(row);
    for (int i = col_dec; i < col_dec+num; i++) {     
            cout << sector[row_dec * 16 + i];
        }
    cout << endl;
}
int ReadBytes2Int(BYTE sector[], const char row[], const char col[], int num) {
    int col_dec = hex2dec(col);
    int row_dec = hex2dec(row);
    int dec = 0;
    int mu = 0;
    for (int i = col_dec ; i <= col_dec + num - 1; i++) {
        if (sector[row_dec * 16 + i] >= 0 && sector[row_dec * 16 + i] <= 15) {
            int temp = (int)sector[row_dec * 16 + i];      
            dec += temp * pow(16, mu);   
        }
        else {
            dec += (int)sector[row_dec * 16 + i] * pow(16, mu);
        }
        mu += 2;
    }
    return dec;
}

int ReadSector(LPCWSTR  drive, int readPoint, BYTE sector[512])
{
    int retCode = 0;
    DWORD bytesRead;
    HANDLE device = NULL;

    device = CreateFile(drive,    // Drive to open
        GENERIC_READ,           // Access mode
        FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
        NULL,                   // Security Descriptor
        OPEN_EXISTING,          // How to create
        0,                      // File attributes
        NULL);                  // Handle to template

    if (device == INVALID_HANDLE_VALUE) // Open Error
    {
        printf("CreateFile: %u\n", GetLastError());
        return 1;
    }

    SetFilePointer(device, readPoint, NULL, FILE_BEGIN);//Set a Point to Read

    if (!ReadFile(device, sector, 512, &bytesRead, NULL))
    {
        printf("ReadFile: %u\n", GetLastError());
    }
    else
    {
       /*
        printf("%hhx ", sector[2*16+12]);
        printf("%hhx ", sector[2 * 16 + 13]);
        printf("%hhx ", sector[2 * 16 + 14]);
        printf("%hhx \n", sector[2 * 16 + 15]);
        for (int i = 0; i < 512; i++) {
            printf("%hhx ", sector[i]);
        }*/
        cout << "\nRead Sector Success!\n";
        cout << "Loai FAT: ";
        ReadBytes2Str(sector, "5" , "2", 8);
        int BS = ReadBytes2Int(sector, "0", "B", 2);
       cout << "So byte cho 1 sector: " << BS << endl;
        int SC = ReadBytes2Int(sector, "0", "D", 1);
        cout << "So sector cho 1 cluster: " << SC << endl;
        int SB = ReadBytes2Int(sector, "0", "E", 2);
        cout << "So sector vung Bootsector: " << SB << endl;
        int NF = ReadBytes2Int(sector, "1", "0", 1);
        cout << "So bang FAT: " << NF << endl;
        int SF = ReadBytes2Int(sector, "2", "4", 4);
        cout << "So sector 1 bang FAT: " << SF << endl;
        int SV = ReadBytes2Int(sector, "2", "0", 4);
        cout << "Tong so sector trong Volume: " << SV << endl;
        cout << "Sector dau tien cua bang FAT1: " << SB << endl;
        int firstSecData = SB + SF * NF;
        cout << "Sector dau tien cua DATA: " << firstSecData << endl;
        
       /* SetFilePointer(device, SB*BS, NULL, FILE_BEGIN);
        DWORD bytereadData;
        BYTE sectorData[512];
        if (ReadFile(device, sectorData, 512, &bytereadData, NULL)) {
            for (int i = 0; i < 512; i++) {
                printf("%hhx ", sectorData[i]);
            }
        }*/
        SetFilePointer(device, firstSecData * 512, NULL, FILE_BEGIN);
        DWORD bytereadData;
        BYTE sectorData[512];
        if (ReadFile(device, sectorData, 512, &bytereadData, NULL)) {
            for (int i = 0; i < 512; i++) {
                printf("%hhx ", sectorData[i]);
            }
        }
    }
}
int main(int argc, char** argv)
{

    BYTE sector[512];
   ReadSector(L"\\\\.\\F:", 0, sector);

   /* BYTE x = '2';
    cout << int(x);*/
    
    
    return 0;
}