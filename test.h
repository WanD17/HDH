#pragma once
#include <windows.h>
#include<iostream>
#include <stdio.h>
#include<vector>
#include<string>
#include<stack>
#include<math.h>
#include <stack>
using namespace std;

struct Entry {
    // Main entry
    BYTE fileName[8];
    BYTE extension[3];
    BYTE Attributes;
    BYTE reserved;
    BYTE createTime_ms;
    //This is file name of sub entry
    BYTE createTime[2];
    BYTE createDate[2];
    BYTE accessedDate[2];
    // 2 bytes in 14
    BYTE startCluster[2];
    BYTE modifiedTime[2];
    BYTE modifiedDate[2];
    // 2 bytes in 1A
    BYTE moreStartCluster[2];
    // size of Main entry/ Last bytes of name in sub entry
    BYTE sizeofFile[4];
};

// Print name of entry
void printName(Entry entry, int mode);
// find Clusters of entry based on FAT
void findCluster(BYTE sectorFAT[512], vector<int>& clusArr, int firstClus);
// find Sectors of entry based on FAT
void findSector(vector<int> clusArr, vector<int>& secArr, int SC, int SB, int NF, int SF);
// Read RDET
void readRDET(HANDLE device, BYTE sectorFAT[512], int rdetPos, int SB, int SC, int NF, int SF);
//Read DATA of .TXT file
void readData(HANDLE device, vector<int> secArr);
// covert hexa 2 decimal
int hex2dec(const char num[]);
// Read bytes and convert 2 string
string ReadBytes2Str(BYTE sector[], const char row[], const char col[], int num);
// Read bytes and convert 2 int
int ReadBytes2Int(BYTE sector[], const char row[], const char col[], int num);

class Format {
public:
    int SB; //sector on Boost sector
    int SC; //sector per cluster
    Format() {
        SB = SC = 0;
    }
	virtual void readBoostSector() = 0;
    virtual void read(HANDLE, BYTE*) = 0;
};

class FAT : public Format {
    BYTE* boostSector;
    int NF; //Number of FAT
    int SF; //Sector per FAT
    
public:
    int RDETpos;
    FAT(BYTE bS[]) {
        boostSector = bS;
        NF = SF = 0;
        RDETpos = 0;
    }
	void readBoostSector() {
        int BS = ReadBytes2Int(boostSector, "0", "B", 2);
        cout << "So byte cho 1 sector: " << BS << endl;
        SC = ReadBytes2Int(boostSector, "0", "D", 1);
        cout << "So sector cho 1 cluster: " << SC << endl;
        SB = ReadBytes2Int(boostSector, "0", "E", 2);
        cout << "So sector vung Bootsector: " << SB << endl;
        int NF = ReadBytes2Int(boostSector, "1", "0", 1);
        cout << "So bang FAT: " << NF << endl;
        int SF = ReadBytes2Int(boostSector, "2", "4", 4);
        cout << "So sector 1 bang FAT: " << SF << endl;
        int SV = ReadBytes2Int(boostSector, "2", "0", 4);
        cout << "Tong so sector trong Volume: " << SV << endl;
        int firstFAT = SB;
        cout << "Sector dau tien cua bang FAT1: " << firstFAT << endl;
        int firstData = SB + SF * NF;
        cout << "Sector dau tien cua DATA: " << firstData << endl;
        int ClusRDET = ReadBytes2Int(boostSector, "2", "C", 4);
        cout << "Cluster dau tien cua RDET: " << ClusRDET << endl;
        RDETpos = firstData + (ClusRDET - 2) * SC;
        cout << "Sector dau tien cua RDET: " << RDETpos << endl;
	}
    void read(HANDLE device, BYTE* s) {
        readRDET(device, s, RDETpos);
    }
    void readRDET(HANDLE, BYTE*, int);
};
class NTFS : public Format {
    int begin_MFT;
    BYTE* boostSector;
public:
    NTFS(BYTE bS[]) {
        boostSector = bS;
        begin_MFT = 0;
    }
    void readBoostSector() {

        int BS = ReadBytes2Int(boostSector, "0","B" , 2);
        cout << "- The number of bytes per sector: " << BS << endl;

        SC = ReadBytes2Int(boostSector, "0", "D", 1);
        cout << "- The number of sectors per cluster: " << SC << endl;

        cout << "- Media descriptor: " << ReadBytes2Int(boostSector, "1", "5", 1) << " hard disk" << endl;

        cout << "- Total number sectors NTFS: " << ReadBytes2Int(boostSector, "2", "8", 8) << endl;

        begin_MFT = ReadBytes2Int(boostSector, "3", "0", 8);
        cout << "- The MFT begin at cluster: " << begin_MFT << endl;

        cout << "- MFT mirror begin at: " << ReadBytes2Int(boostSector, "3","8", 8) << endl;
    }
    void read(HANDLE d, BYTE* s) {

    }
};
void FAT::readRDET(HANDLE device, BYTE sectorFAT[512], int RDETpos) {

    DWORD BytesRDETRead;
    Entry entry;
    BYTE bytesEntry[512];
    memset(&bytesEntry, 0, 512);

    if (device != NULL) {
        SetFilePointer(device, (512 * RDETpos), NULL, FILE_BEGIN);
        bool isEmpty = false;
        do {
            if (!ReadFile(device, bytesEntry, 512, &BytesRDETRead, NULL)) {
                cout << "Error in Reading RDET.\n";
                return;
            }
            else {
                BYTE* pEntry = bytesEntry;

                // Read 1 entry = 32 bytes
                for (int i = 0; i < (512 / 32); i++) {
                    memcpy(&entry, pEntry, 32);
                    bool hasSubEntry = false;

                    // check entry is empty (first bytes is 0x00)
                    if (entry.fileName[0] == 0x00) {
                        //if Empty entry that means there is no more entry to read
                        isEmpty = true;
                        break;
                    }
                    else {
                        // if entry is deleted(first byte is 0xE5), read next entry
                        if (entry.fileName[0] == 0xE5) {
                            pEntry += 32;
                            continue;
                        }
                        // if . and .. entry, read next entry
                        if (entry.fileName[0] == 0x2E) {
                            pEntry += 32;
                            continue;
                        }
                        //ignore some entry
                        if (entry.Attributes != 0x10 && entry.Attributes != 0x20 && entry.Attributes != 0x0F && entry.Attributes != 0x04
                            && entry.Attributes != 0x02 && entry.Attributes != 0x01 && entry.Attributes != 0x08) {
                            pEntry += 32;
                            continue;
                        }
                        //check entry is sub entry
                        if (entry.Attributes == 0x0F) {
                            // if sub entry, push to stack
                            stack<Entry> subEntries;
                            hasSubEntry = 1;

                            //Read while next entry is sub entry
                            while (entry.Attributes == 0x0F) {
                                subEntries.push(entry);
                                pEntry += 32;
                                i++;
                                memcpy(&entry, pEntry, 32);
                            };
                            // After find sub entry, print name of them
                            cout << "\n\nFile name: ";
                            do {
                                Entry subEntry = subEntries.top();
                                subEntries.pop();
                                printName(subEntry, 1);
                            } while (!subEntries.empty());
                        }

                        //Main entry name
                        if (!hasSubEntry) {
                            cout << "\n\nFile Name: ";
                            printName(entry, 0);
                        }
                        cout << endl;

                        // attribute of entry (1 bytes in 0B)
                        if (entry.Attributes == 0x01)
                            cout << "File Attribute: Read Only File\n";
                        else if (entry.Attributes == 0x02)
                            cout << "File Attribute: Hidden File\n";
                        else if (entry.Attributes == 0x04)
                            cout << "File Attribute: System File\n";
                        else if (entry.Attributes == 0x08)
                            cout << "File Attribute: Volume Label\n";
                        else if (entry.Attributes == 0x10)
                            cout << "File Attribute: Directory\n";
                        else if (entry.Attributes == 0x20)
                            cout << "File Attribute: Archive\n";

                        // first Cluster (2bytes in 14 + 2bytes in 1A)
                        int upCluster = entry.startCluster[1] << 8 | entry.startCluster[0];
                        int  downCluster = entry.moreStartCluster[1] << 8 | entry.moreStartCluster[0];
                        int startCluster = upCluster + downCluster;
                        cout << "Start Cluster: " << startCluster << endl;
                        // find clusters
                        vector<int> clusArray;
                        findCluster(sectorFAT, clusArray, startCluster);
                        cout << "In Cluster: ";
                        for (int i = 0; i < clusArray.size(); i++) {
                            cout << clusArray[i] << " ";
                        }
                        // find sector
                        cout << "\nIn Sector: ";
                        vector<int> secArray;
                        findSector(clusArray, secArray, SC, SB, NF, SF);
                        for (int i = 0; i < secArray.size(); i++) {
                            cout << secArray[i] << " ";
                        }
                        // Size of file
                        int size = 0;
                        for (int i = 3; i >= 0; i--)
                            size = size << 8 | entry.sizeofFile[i];
                        cout << "\nSize: " << size << " bytes\n";
                        //if entry is .txt file, read data
                        if (entry.extension[0] == 'T' && entry.extension[1] == 'X' && entry.extension[2] == 'T') {
                            readData(device, secArray);
                        }
                        else if (entry.Attributes != 0x10) {
                            cout << "-> Dung phan mem tuong thich de doc noi dung!" << endl;
                        }
                        if (entry.Attributes == 0x10) {
                            readRDET(device, sectorFAT, secArray[0]);
                        }
                        //update pEntry
                        pEntry += 32;

                        cout << endl;
                        cout << "--------------------------------------------------------------------------------------" << endl;
                        cout << endl;
                    }
                }
            }
            if (isEmpty)
                break;
        } while (1);
    }
    else {
        cout << "Error in open disk! ";
        return;
    }

}
void findSector(vector<int> clusArr, vector<int>& secArr, int SC, int SB, int NF, int SF) {
    for (int i = 0; i < clusArr.size(); i++) {
        //find first sector based on i = SB+NF*SF+(k-2)*SC with k is cluster index
        int secPos = SB + NF * SF + (clusArr[i] - 2) * SC;
        for (int i = 0; i < SC; i++) {
            //push to vector
            secArr.push_back(secPos + i);
        }
    }
}
void findCluster(BYTE sectorFAT[512], vector<int>& clusArr, int firstClus) {
    clusArr.push_back(firstClus);
    int next = firstClus;
    while (1) {
        int temp[4];
        for (int i = 0; i < 4; i++) {
            temp[i] = (int)sectorFAT[next * 4 + i];
        }
        // if FAT[i] is EOF then break
        if ((temp[0] == 255 || temp[0] == 248) && temp[1] == 255 && temp[2] == 255 && temp[3] == 15) {
            break;
        }
        // calc next cluster
        next = 0;
        int mu = 0;
        for (int i = 0; i <= 3; i++) {
            next += temp[i] * pow(16, mu);
            mu++;
        }

        cout << endl;
        //push cluster to vector
        clusArr.push_back(next);
    }
}
void printName(Entry entry, int mode) {
    //mode 0 is Main entry
    if (mode == 0) {
        int i = 0;
        while (entry.fileName[i] != 0x20 && i < 8) cout << entry.fileName[i++];

        if (entry.extension[0] != 0x20) {
            cout << ".";
            for (i = 0; i < 3; i++)
                cout << entry.extension[i];
        }
    }
    // Sub entry
    else {
        for (int i = 1; i < 8; i += 2)
            cout << entry.fileName[i];
        cout << entry.extension[1];
        cout << entry.createTime[0] << entry.createDate[0]
            << entry.accessedDate[0] << entry.startCluster[0]
            << entry.modifiedTime[0] << entry.modifiedDate[0];
        for (int i = 0; i < 4; i += 2)
            cout << entry.sizeofFile[i];
    }
}
int ReadBytes2Int(BYTE sector[], const char row[], const char col[], int num) {
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
string ReadBytes2Str(BYTE sector[], const char row[], const char col[], int num) {
    int col_dec = hex2dec(col);
    int row_dec = hex2dec(row);
    string result = "";
    for (int i = col_dec; i < col_dec + num; i++) {
        result += sector[row_dec * 16 + i];
    }
    return result;
}
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