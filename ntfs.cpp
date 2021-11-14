// nhoxoa.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include <set>
#include <queue>
#include <math.h>
#include <cstdlib>
#include <fstream>
#include <iomanip>
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

int ReadBytes2Int(byte sector[], const char row[], const char col[], int num) {
    int col_dec = hex2dec(col);
    int row_dec = hex2dec(row);
    int dec = 0;
    int mu = 0;
    for (int i = col_dec; i <= col_dec + num - 1; i++) {
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

string Bytes2Str(BYTE sector[], int pos, int num) {
    string result = "";
    while (num > 0) {
        result += sector[pos];
        num--;
        pos++;
    }
    return result;
}

int Byte2Int(BYTE sector[], int pos, int num) {
    int dec = 0;
    int mu = 0;
    while (num > 0) {
        if (sector[pos] >= 0 && sector[pos] <= 15) {
            int temp = (int)sector[pos];
            dec += temp * pow(16, mu);
        }
        else {
            dec += (int)sector[pos] * pow(16, mu);
        }
        mu += 2;
        num--;
        pos++;
    }
    return dec;
}



struct FILENAME  //luu cac thong so can thiet cua 1 entry
{
    int indexSector;
    int ID;
    int IDparent;
    int attribute;
    string name;
    string data;
};

ostream& operator << (ostream& out, FILENAME f) // phu tro xuat TapTin/ThuMuc
{
    int i = f.name.length();
    //xuat data neu file txt
    if (((int)f.name[i - 2]) == 116 && ((int)f.name[i - 4]) == 120 && ((int)f.name[i - 6]) == 116)
        out << f.name << " (sector: " << f.indexSector << " ; Noi dung: " << f.data << ")\n";
    else if (f.attribute == 268435456) // thu muc
        out << f.name << " (sector: " << f.indexSector << ")\n";
    else
        out << f.name << " (sector: " << f.indexSector << ")" << " => Hay mo tap tin bang ung dung tuong thich\n";
    return out;
}


int ReadDrive(LPCWSTR  drive);
FILENAME readEntry(BYTE sector[1024]);
int readAttribute(BYTE sector[1024]);
int readMFT(HANDLE device, int posMFT);
void print_RootFolder(queue<FILENAME>& src, int IDparent, int n_setw);



int main(int argc, char** argv)
{

    ReadDrive(L"\\\\.\\F:");
    
    return 0;

}

//FUCTIONS:
int ReadDrive(LPCWSTR  drive) //doc o dia
{
    DWORD bytesRead;
    HANDLE device = NULL;
    BYTE sector[512];
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

    if (!ReadFile(device, sector, 512, &bytesRead, NULL))
    {
        printf("ReadFile: %u\n", GetLastError());
    }
    else
    {

        cout << "Read Boost Sector Success!\n\n";
        cout << "- The number of bytes per sector: " << ReadBytes2Int(sector, "0", "B", 2) << endl;
        int Sc = ReadBytes2Int(sector, "0", "D", 1);
        cout << "- The number of sectors per cluster: " << Sc << endl;
        cout << "- Total number sectors NTFS: " << ReadBytes2Int(sector, "2", "8", 8) << endl;
        int cMFT = ReadBytes2Int(sector, "3", "0", 8);
        cout << "- The MFT begin at cluster: " << cMFT << endl;
        cout << "- MFT mirror begin at: " << ReadBytes2Int(sector, "3", "8", 8) << endl;
        system("pause");
        system("cls");

        //doc tap tin/thu muc
        readMFT(device, Sc * cMFT);


    }
    CloseHandle(device);
}

FILENAME readEntry(BYTE sector[1024]) // Lay cac thong so can thiet cua entry
{
    FILENAME filename;
    filename.ID = ReadBytes2Int(sector, "2", "C", 4);
    int posSTAND = ReadBytes2Int(sector, "1", "4", 2);
    int sizeSTAND = Byte2Int(sector, posSTAND + 4, 4);
    int sizeFILENAME = Byte2Int(sector, posSTAND + sizeSTAND + 4, 4);
    int posDataFILE = Byte2Int(sector, posSTAND + sizeSTAND + 20, 2);
    int sizeDataFILE = Byte2Int(sector, posSTAND + sizeSTAND + 16, 4);
    filename.IDparent = Byte2Int(sector, posSTAND + sizeSTAND + posDataFILE, 5);
    filename.name = Bytes2Str(sector, posSTAND + sizeSTAND + posDataFILE + 66, sizeDataFILE - 66);
    int posOBJ = posSTAND + sizeSTAND + sizeFILENAME;
    int sizeOBJ = Byte2Int(sector, posOBJ + 4, 4);
    int posDataDATA = Byte2Int(sector, posOBJ + sizeOBJ + 20, 2);
    int sizeDataDATA = Byte2Int(sector, posOBJ + sizeOBJ + 16, 4);
    string data = "";
    while (sector[posOBJ + sizeOBJ + posDataDATA] != 0xFF) {
        data += sector[posOBJ + sizeOBJ + posDataDATA];
        posDataDATA++;
    }
    filename.data = data;
    return filename;
}

int readAttribute(BYTE sector[1024]) //doc attribute cua entry de xac dinh ThuMuc/TapTin
{
    int posSTAND = ReadBytes2Int(sector, "1", "4", 2);
    int sizeSTAND = Byte2Int(sector, posSTAND + 4, 4);
    int posDataFILE = Byte2Int(sector, posSTAND + sizeSTAND + 20, 2);
    return Byte2Int(sector, posSTAND + sizeSTAND + posDataFILE + 56, 4);
}

int readMFT(HANDLE device, int posMFT)  // doc bang MFT (duyet cac entry co trong MFT)
{
    if (device != NULL) {
        BYTE sectorMFTentry[1024];
        DWORD BytesMFTentry;
        int i = 0;
        queue<FILENAME> MFTentries; // luu entry thu muc va tap tin
        while (i < 200) // ko biet diem dung cua MFT => duyet tam 100 entry
        {
            SetFilePointer(device, (512 * (posMFT + i)), NULL, FILE_BEGIN);
            if (!ReadFile(device, sectorMFTentry, 1024, &BytesMFTentry, NULL)) {
                cout << "Error in Reading MFT";
                return 0;
            }
            int attr = readAttribute(sectorMFTentry);
            if (attr == 268435456 || attr == 32) // 0x00000010 or 0x20000000   Tap Tin or Thu muc
            {
                FILENAME f = readEntry(sectorMFTentry);
                f.indexSector = posMFT + i;
                f.attribute = attr;
                MFTentries.push(f);
            }
            i += 2;
        }
        cout << "Cay thu muc goc: \n\n";
        print_RootFolder(MFTentries, 5, 0);
    }
    return 1;
}

void print_RootFolder(queue<FILENAME>& src, int IDparent, int n_setw) //in cay thu muc goc
{
    int k = src.size();
    while (k) {
        FILENAME temp = src.front();
        if (temp.IDparent == IDparent) {
            if (temp.attribute == 268435456)//thu muc
            {
                cout << setw(n_setw) << left << " " << "+ " << temp;
                src.pop();
                print_RootFolder(src, temp.ID, n_setw + 5);
                k = src.size();
                continue;
            }
            else {
                src.pop();
                cout << setw(n_setw) << left << " " << "- " << temp;
            }
        }
        src.push(src.front());
        src.pop();
        k--;
    }
}