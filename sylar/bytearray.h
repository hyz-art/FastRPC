#ifndef __SYLAR_BYTEARRAY_H__
#define __SYLAR_BYTEARRAY_H__
#include <stdint.h>
#include <memory>
#include <string>
#include <vector>
#include <sys/uio.h>
namespace sylar {

class ByteArray {
public:
    typedef std::shared_ptr<ByteArray> ptr;

    struct Node {
        Node(size_t s);
        Node();
        ~Node();

        char* ptr;
        Node* next;
        size_t size;
    };

    ByteArray(size_t base_size = 4096);
    ~ByteArray();

    // write
    void writeFint8  (int8_t value);
    void writeFuint8 (uint8_t value);
    void writeFint16 (int16_t value);
    void writeFuint16(uint16_t value);
    void writeFint32 (int32_t value);
    void writeFuint32(uint32_t value);
    void writeFint64 (int64_t value);
    void writeFuint64(uint64_t value);

    void writeInt32 (int32_t value);
    void writeUint32(uint32_t value);
    void writeInt64 (int64_t value);
    void writeUint64(uint64_t value);

    void writeFloat(float value);
    void writeDouble(double value);
    // length:int16
    void writeStringF16(const std::string& value);
    // length:int32
    void writeStringF32(const std::string& value);
    // length:int64
    void writeStringF64(const std::string& value);
    // length:vint
    void writeStringVint(const std::string& value);
    // data
    void writeStringWithoutLength(const std::string& value);

    //read
    int8_t readFint8();
    uint8_t readFuint8();
    int16_t readFint16();
    uint16_t readFuint16();
    int32_t readFint32();
    uint32_t readFuint32();
    int64_t readFint64();
    uint64_t readFuint64();

    int32_t readInt32();
    uint32_t readUint32();
    int64_t readInt64();
    uint64_t readUint64();

    float readFloat();
    double readDouble();

    std::string readStringF16();
    std::string readStringF32();
    std::string readStringF64();
    std::string readStringVint();

    void clear();
    void write(const void* buf, size_t size);
    void read(void* buf, size_t size);
    void read(void* buf, size_t size, size_t position)const;
    
    size_t getPosition()const { return m_position; }
    void setPosition(size_t v);

    bool writeToFile(const std::string& name)const;
    bool readFromFile(const std::string& name);

    size_t getBaseSize()const { return m_basesize; }
    size_t getReadSize()const { return m_size-m_position; }
    size_t getSize()const { return m_size; }
    //设置小端
    bool isLittleEndian()const;
    void setIsLittleEndian(bool val);
    //转成string类型
    std::string toString()const;
    //将ByteArray里面的数据[m_position, m_size)转成16进制的std::string(格式:FF FF FF)
    std::string toHexString()const;

    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len=~0ull)const;
    uint64_t getReadBuffers(std::vector<iovec>& buffers,uint64_t len, uint64_t position)const;
    uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);
private:
    void addCapacity(size_t size);
    size_t getCapacity()const { return m_capacity-m_position; }
private:
    
    //容量大小
    size_t m_basesize;
    //当前操作位置
    size_t m_position;
    //、总容量、数据大小
    size_t m_capacity;
    size_t m_size;
    
    //字节序
    int8_t m_endian;
    //内存块指针
    Node* m_root;
    Node* m_cur;
};

}

#endif