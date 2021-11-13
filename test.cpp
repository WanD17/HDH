#include "test.h"




int ReadDrive(LPCWSTR  drive)
{
    DWORD bytesRead;
    HANDLE device = NULL;
    BYTE boostSector[512];
    int readPoint = 0;

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

    if (!ReadFile(device, boostSector, 512, &bytesRead, NULL))
    {
        printf("ReadFile: %u\n", GetLastError());
    }
    else
    {
        //Read Boost sector to find SV,SC,SB,NF,SF,....
        cout << "Read Boost Sector Success!\n\n";
        string format = ReadBytes2Str(boostSector, "5", "2", 8);
        Format* F;
        cout << format << endl;;
        if (format.find("FAT") != std::string::npos)
        {
            cout << "Loai FAT: " << format << endl;
            F = new FAT(boostSector);
        }
        else {
            cout << "NTFS: \n";
            F = new NTFS(boostSector);
        }
        F->readBoostSector();
        system("pause");
        system("cls");

        BYTE sectorFAT[512];
        DWORD bytesFat;
        SetFilePointer(device, (512 * F->SB), NULL, FILE_BEGIN);
        if (!ReadFile(device, sectorFAT, 512, &bytesFat, NULL)) {
            cout << "Error in Reading FAT";
            return 0;
        }

        F->read(device, sectorFAT);
    }
    CloseHandle(device);
}
int main(int argc, char** argv)
{
   ReadDrive(L"\\\\.\\F:");

    return 0;
}  




void readData(HANDLE device, vector<int> secArr) {
    DWORD bytesDATARead;
    BYTE DataSector[512];
    memset(&DataSector, 0, 512);
    if (device == NULL) // Error
    {
        printf("Error in read drive");
        return;
    }
    else
    {
        cout << "Noi dung tap tin: ";
        for (int i = 0; i < secArr.size(); i++) {
            SetFilePointer(device, (512 * secArr[i]), NULL, FILE_BEGIN);

            if (!ReadFile(device, DataSector, 512, &bytesDATARead, NULL)) {
                printf("\nReadFile: %u\n", GetLastError());
            }
            else {
                
                int i = 0;
                while (DataSector[i] != 0x00) {
                    cout << DataSector[i++];

                }
            }
        }
    }
}
