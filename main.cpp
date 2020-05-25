#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

class BinWriter {
public:
    int k;
    ofstream f;
    char x;

    BinWriter(const char *p) : k(0) {
        f.open(p, ios::binary);
    }

    ~BinWriter() {
        if (k > 0) writeByte(x);
        f.close();
    }

    void writeByte(char x) {
        f.write((char *) &x, 1);
    }

    void writeInt(int y) {
        f.write((char *) &y, 4);
    }

    void writeBit(bool b) {
        if (k == 8) {
            writeByte(x);
            k = 0;
        }
        x ^= (-b ^ x) & (1 << k);
        k++;
    }
};

class BinReader {
public:
    int k, l;
    ifstream f;
    char x;

    BinReader(const char *p) : k(0) {
        f.open(p, ios::binary);
    }

    char readByte() {
        f.read((char *) &x, 1);
        return x;
    }

    bool readBit() {
        if (k == 8) {
            readByte();
            k = 0;
        }
        bool b = (x >> k) & 1;
        k++;
        return b;
    }

    int readInt() {
        f.read((char *) &l, 4);
        return l;
    }
};

struct character {
    char character = ' ';
    int number = 0;
    vector<bool> bytes;
};

void division(character *charTable, int eleNum, int x) {
    if (eleNum != x) {
        int left = x, right = eleNum;
        int moveLeft = charTable[x].number;
        int moveRight = charTable[eleNum].number;

        while (x + 1 != eleNum && eleNum - 1 != x) {
            if (moveLeft > moveRight) {
                eleNum--;
                moveRight += charTable[eleNum].number;
            } else {
                x++;
                moveLeft += charTable[x].number;
            }
        }

        for (int i = left; i <= x && charTable[i].number != 0; i++) charTable[i].bytes.push_back(0);
        for (int i = eleNum; i <= right && charTable[i].number != 0; i++) charTable[i].bytes.push_back(1);

        division(charTable, x, left);
        division(charTable, right, eleNum);
    }
}

int numOfBytes(character *charTable) {
    int sum = 0;
    for (int i = 0; i < 256 && charTable[i].number != 0; i++) {
        sum += charTable[i].bytes.size() * charTable[i].number;
    }

    return sum;
}

void write(character *charTable, string str, unsigned int length) {
    BinWriter writer("out.bin");
    for (int x = 0; x < 256; x++) {
        writer.writeByte(charTable[x].character);
        writer.writeInt(charTable[x].number);
    }

    int toFind;
    for (unsigned int x = 0; x < length; x++) {
        for (vector<bool>::iterator it = charTable[toFind].bytes.begin();
             it != charTable[toFind].bytes.end(); ++it) {
            writer.writeBit(*it);
        }
    }
    writer.f.close();
}

void compression(character *charTable, string str) {
    for (unsigned int x = 0; x < str.length(); x++) {
        charTable[(int) str[x]].number++;
        charTable[(int) str[x]].character = str[x];
    }

    for (int x = 0; x < 255; x++) {
        for (int i = x; i < 255; i++) {
            if (charTable[x].number < charTable[i].number) {
                character temp = charTable[x];
                charTable[x] = charTable[i];
                charTable[i] = temp;
            }
        }
    }

    division(charTable, 255, 0);
}

void decompression(character *charTable) {
    BinReader reader("out.bin");
    for (int x = 0; x < 256; x++) {
        charTable[x].character = reader.readByte();
        charTable[x].number = reader.readInt();
    }

    division(charTable, 255, 0);
    for (int x = 0; x < 256; x++) {
        for (int i = x; i < 256; i++) {
            if (charTable[x].bytes.size() < charTable[i].bytes.size()) {
                character temp = charTable[x];
                charTable[x] = charTable[i];
                charTable[i] = temp;
            }
        }
    }

    bool buffer[charTable[0].bytes.size()];
    reader.readByte();

    int allBytes = numOfBytes(charTable);
    for (unsigned int x = 0; x < charTable[0].bytes.size(); x++) {
        buffer[x] = reader.readBit();
    }

    bool out;
    for (int i = 0, x = charTable[0].bytes.size(); x < allBytes; i++) {
        int y = 0;
        for (vector<bool>::iterator it = charTable[i].bytes.begin(); it != charTable[i].bytes.end(); ++it) {
            if (buffer[y] != *it) {
                out = false;
                break;
            } else out = true;
            y++;
        }
        if (out) {
            if (charTable[i].bytes.size() == charTable[0].bytes.size()) {
                for (unsigned int x = 0; x < charTable[0].bytes.size(); x++) {
                    buffer[x] = reader.readBit();
                }
                x += charTable[i].bytes.size();
            } else {
                for (unsigned int level = charTable[i].bytes.size(); level < charTable[0].bytes.size(); level++) {
                    buffer[level - charTable[i].bytes.size()] = buffer[level];
                }

                for (unsigned int fill = charTable[0].bytes.size() - charTable[i].bytes.size();
                     fill < charTable[0].bytes.size(); fill++) {
                    buffer[fill] = reader.readBit();
                }
                x += charTable[i].bytes.size();
            }
            i = 0;
        }
    }
}

int main(int argc, char **argv) {
    if (argv[1][0] == 'c') {
        character charTable[256];
        ifstream file(argv[2]);
        string str;

        file.seekg(0, ios::end);
        str.reserve(file.tellg());
        file.seekg(0, ios::beg);
        str.assign((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

        compression(charTable, str);
        write(charTable, str, str.length());
    } else if (argv[1][0] == 'd') {
        character charTable[256];
        ifstream file("out.bin");
        string str;

        str.reserve(file.tellg());
        decompression(charTable);
        write(charTable, str, str.length());
    }

    return 0;
}