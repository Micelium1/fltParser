#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <iomanip>
#include <cstdlib>
using namespace std;

#pragma pack(push, 1) 

template <typename T>
T swap_endian(T value) {
    static_assert(std::is_integral_v<T>, "swap_endian works only with integer types");

    union {
        T value;
        uint8_t bytes[sizeof(T)];
    } converter;

    converter.value = value;


    for (size_t i = 0; i < sizeof(T) / 2; ++i) {
        std::swap(converter.bytes[i], converter.bytes[sizeof(T) - 1 - i]);
    }

    return converter.value;
}


struct RecordHeader {
    int16_t opcode;
    uint16_t length;
};


struct HeaderRecord {
    RecordHeader header;
    char id[8];
};


struct GroupRecord {
    RecordHeader header;
    char id[8];
};


struct ObjectRecord {
    RecordHeader header;
    char id[8];
};

struct FaceRecord {
    RecordHeader header;
    char id[8];                        
    int32_t irColorCode;               
    int16_t relativePriority;
    int8_t drawType;                   
    int8_t textureWhite;               
    uint16_t colorNameIndex;           
    uint16_t altColorNameIndex;        
    int8_t reserved1;
    int8_t templateBillboard;          
    int16_t detailTexturePatternIndex; 
    int16_t texturePatternIndex;       
    int16_t materialIndex;

};

struct PushLevelRecord {
    RecordHeader header;
};

struct PopLevelRecord {
    RecordHeader header;
};

#pragma pack(pop) 




class OpenFlightParser {

public:

    bool parseFile(const string& filename) {
        ifstream file(filename, ios::binary);
        if (!file) {
            cerr << "Error opening file: " << filename << endl;
            return false;
        }

        while (file) {
            RecordHeader header;
            file.read(reinterpret_cast<char*>(&header), sizeof(header));

            if (file.gcount() == 0) break; 
            header.opcode = swap_endian(header.opcode);
            header.length = swap_endian(header.length);
            switch (header.opcode) {
            case 1: 
                parseHeader(file, header);
                break;
            case 2: 
                parseGroup(file, header);
                break;
            case 4: 
                parseObject(file, header);
                break;
            case 5: 
                parseFace(file, header);
                break;
            case 10: 
                parsePushLevel(file, header);
                break;
            case 11: 
                parsePopLevel(file, header);
                break;
            case 33: 
            {
                char* fullName = new char[header.length - 4];
                file.read(fullName, header.length - 4);
                printWithIndent(fullName, "Full ID");
            }
                break;
            default:
                file.seekg(header.length - sizeof(header), ios::cur);
                break;
            }
        }

        return true;
    }

private:
    int currentLevel = 0;

    void parseHeader(ifstream& file, const RecordHeader& header) {
        HeaderRecord hdr;
        hdr.header = header;
        file.read(reinterpret_cast<char*>(&hdr) + sizeof(hdr.header),
            sizeof(hdr) - sizeof(RecordHeader));

        printWithIndent(string(hdr.id), "Header");

        int remaining = header.length - sizeof(hdr);
        if (remaining > 0) {
            file.seekg(remaining, ios::cur);
        }
    }

    void parseGroup(ifstream& file, const RecordHeader& header) {
        GroupRecord group;
        file.read(reinterpret_cast<char*>(&group) + sizeof(RecordHeader),
            sizeof(group) - sizeof(RecordHeader));

        printWithIndent(string(group.id), "Group");

        int remaining = header.length - sizeof(group);
        if (remaining > 0) {
            file.seekg(remaining, ios::cur);
        }
    }

    void parseObject(ifstream& file, const RecordHeader& header) {
        ObjectRecord obj;
        file.read(reinterpret_cast<char*>(&obj) + sizeof(RecordHeader),
            sizeof(obj) - sizeof(RecordHeader));

        printWithIndent(string(obj.id), "Object");

        int remaining = header.length - sizeof(obj);
        if (remaining > 0) {
            file.seekg(remaining, ios::cur);
        }
    }

    void parseFace(ifstream& file, const RecordHeader& header) {
        FaceRecord face;
        face.header = header;
        file.read(reinterpret_cast<char*>(&face) + sizeof(RecordHeader),
            sizeof(face) - sizeof(RecordHeader));
        face.colorNameIndex = swap_endian(face.colorNameIndex);
        face.materialIndex = swap_endian(face.materialIndex);
        string info = string(face.id) + " (Material Index: " +
            to_string(face.materialIndex) +" (Color Index : " +
            to_string(face.colorNameIndex) +")";
        printWithIndent(info, "Face");

        int remaining = header.length - sizeof(face);
        if (remaining > 0) {
            file.seekg(remaining, ios::cur);
        }
    }

    void parsePushLevel(ifstream& file, const RecordHeader& header) {
        currentLevel++;
    }

    void parsePopLevel(ifstream& file, const RecordHeader& header) {
        currentLevel--;
    }

    void printWithIndent(const string& text, const string& type) {
        cout << string(currentLevel * 2, ' ') << type << ": " << text << endl;
    }
};

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "");
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <flt_file>" << endl;
        return 1;
    }

    OpenFlightParser parser;
    if (!parser.parseFile(argv[1])) {
        return 2;
    }
    system("pause");
    return 0;
}