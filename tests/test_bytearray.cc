#include "sylar/bytearray.h"
#include "sylar/sylar.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test() {
#define XX(type, len, write_fun, read_fun, base_len)                                                                 \
    {                                                                                                                \
        std::vector<type> vec;                                                                                       \
        for (int i = 0; i < len; ++i) {                                                                              \
            vec.push_back(rand());                                                                                   \
        }                                                                                                            \
        sylar::ByteArray::ptr ba(new sylar::ByteArray(base_len));                                                    \
        for (auto& i : vec) {                                                                                        \
            ba->write_fun(i);                                                                                        \
        }                                                                                                            \
        ba->setPosition(0);                                                                                          \
        for (size_t i = 0; i < vec.size(); ++i) {                                                                    \
            type v = ba->read_fun();                                                                                 \
            SYLAR_ASSERT(v == vec[i]);                                                                               \
        }                                                                                                            \
        SYLAR_ASSERT(ba->getReadSize() == 0);                                                                        \
        SYLAR_LOG_INFO(g_logger) << #write_fun "/" #read_fun " (" #type " ) len=" << len << " base_len=" << base_len \
                                 << " size=" << ba->getSize();                                                       \
    }

    XX(int8_t, 100, writeFint8, readFint8, 1);
    XX(uint8_t, 100, writeFuint8, readFuint8, 1);
    XX(int16_t, 100, writeFint16, readFint16, 1);
    XX(uint16_t, 100, writeFuint16, readFuint16, 1);
    XX(int32_t, 100, writeFint32, readFint32, 1);
    XX(uint32_t, 100, writeFuint32, readFuint32, 1);
    XX(int64_t, 100, writeFint64, readFint64, 1);
    XX(uint64_t, 100, writeFuint64, readFuint64, 1);

    XX(int32_t, 100, writeInt32, readInt32, 1);
    XX(uint32_t, 100, writeUint32, readUint32, 1);
    XX(int64_t, 100, writeInt64, readInt64, 1);
    XX(uint64_t, 100, writeUint64, readUint64, 1);
#undef XX

#define XX(type, len, write_fun, read_fun, base_len)                                                                 \
    {                                                                                                                \
        std::vector<type> vec;                                                                                       \
        for (int i = 0; i < len; ++i) {                                                                              \
            vec.push_back(rand());                                                                                   \
        }                                                                                                            \
        sylar::ByteArray::ptr ba(new sylar::ByteArray(base_len));                                                    \
        for (auto& i : vec) {                                                                                        \
            ba->write_fun(i);                                                                                        \
        }                                                                                                            \
        ba->setPosition(0);                                                                                          \
        for (size_t i = 0; i < vec.size(); ++i) {                                                                    \
            type v = ba->read_fun();                                                                                 \
            SYLAR_ASSERT(v == vec[i]);                                                                               \
        }                                                                                                            \
        SYLAR_ASSERT(ba->getReadSize() == 0);                                                                        \
        SYLAR_LOG_INFO(g_logger) << #write_fun "/" #read_fun " (" #type " ) len=" << len << " base_len=" << base_len \
                                 << " size=" << ba->getSize();                                                       \
        ba->setPosition(0);                                                                                          \
        SYLAR_ASSERT(ba->writeToFile("/tmp/" #type "_" #len "-" #read_fun ".dat"));                                  \
        sylar::ByteArray::ptr ba2(new sylar::ByteArray(base_len * 2));                                               \
        SYLAR_ASSERT(ba2->readFromFile("/tmp/" #type "_" #len "-" #read_fun ".dat"));                                \
        ba2->setPosition(0);                                                                                         \
        SYLAR_ASSERT(ba->toString() == ba2->toString());                                                             \
        SYLAR_ASSERT(ba->getPosition() == 0);                                                                        \
        SYLAR_ASSERT(ba2->getPosition() == 0);                                                                       \
    }
    XX(int8_t, 100, writeFint8, readFint8, 1);
    XX(uint8_t, 100, writeFuint8, readFuint8, 1);
    XX(int16_t, 100, writeFint16, readFint16, 1);
    XX(uint16_t, 100, writeFuint16, readFuint16, 1);
    XX(int32_t, 100, writeFint32, readFint32, 1);
    XX(uint32_t, 100, writeFuint32, readFuint32, 1);
    XX(int64_t, 100, writeFint64, readFint64, 1);
    XX(uint64_t, 100, writeFuint64, readFuint64, 1);

    XX(int32_t, 100, writeInt32, readInt32, 1);
    XX(uint32_t, 100, writeUint32, readUint32, 1);
    XX(int64_t, 100, writeInt64, readInt64, 1);
    XX(uint64_t, 100, writeUint64, readUint64, 1);

#undef XX
}


void testExtended() {
#define XX(type, len, write_fun, read_fun, base_len)                                                                 \
    {                                                                                                                \
        std::vector<type> vec;                                                                                       \
        for (int i = 0; i < len; ++i) {                                                                              \
            vec.push_back(rand());                                                                                   \
        }                                                                                                            \
        sylar::ByteArray::ptr ba(new sylar::ByteArray(base_len));                                                    \
        for (auto& i : vec) {                                                                                        \
            ba->write_fun(i);                                                                                        \
        }                                                                                                            \
        ba->setPosition(0);                                                                                          \
        for (size_t i = 0; i < vec.size(); ++i) {                                                                    \
            type v = ba->read_fun();                                                                                 \
            SYLAR_ASSERT(v == vec[i]);                                                                               \
        }                                                                                                            \
        SYLAR_ASSERT(ba->getReadSize() == 0);                                                                        \
        SYLAR_LOG_INFO(g_logger) << #write_fun "/" #read_fun " (" #type " ) len=" << len << " base_len=" << base_len \
                                 << " size=" << ba->getSize();                                                       \
    }

#define XX_FILE(type, len, write_fun, read_fun, base_len)                                                            \
    {                                                                                                                \
        std::vector<type> vec;                                                                                       \
        for (int i = 0; i < len; ++i) {                                                                              \
            vec.push_back(static_cast<type>(rand()));                                                                \
        }                                                                                                            \
        sylar::ByteArray::ptr ba(new sylar::ByteArray(base_len));                                                    \
        for (auto& i : vec) {                                                                                        \
            ba->write_fun(i);                                                                                        \
        }                                                                                                            \
        ba->setPosition(0);                                                                                          \
        for (size_t i = 0; i < vec.size(); ++i) {                                                                    \
            type v = ba->read_fun();                                                                                 \
            SYLAR_ASSERT(v == vec[i]);                                                                               \
        }                                                                                                            \
        SYLAR_ASSERT(ba->getReadSize() == 0);                                                                        \
        SYLAR_LOG_INFO(g_logger) << #write_fun "/" #read_fun " (" #type " ) len=" << len << " base_len=" << base_len \
                                 << " size=" << ba->getSize();                                                       \
        ba->setPosition(0);                                                                                          \
        std::string filename = "/tmp/" #type "_" #len "-" #read_fun ".dat";                                          \
        SYLAR_ASSERT(ba->writeToFile(filename));                                                                     \
        sylar::ByteArray::ptr ba2(new sylar::ByteArray(base_len * 2));                                               \
        SYLAR_ASSERT(ba2->readFromFile(filename));                                                                   \
        ba2->setPosition(0);                                                                                         \
        SYLAR_ASSERT(ba->toString() == ba2->toString());                                                             \
        SYLAR_ASSERT(ba->getPosition() == 0);                                                                        \
        SYLAR_ASSERT(ba2->getPosition() == 0);                                                                       \
    }

    // ==================== 浮点数测试 ====================
    XX(float, 100, writeFloat, readFloat, 1);
    XX(double, 100, writeDouble, readDouble, 1);

    XX_FILE(float, 100, writeFloat, readFloat, 1);
    XX_FILE(double, 100, writeDouble, readDouble, 1);

    // ==================== 布尔类型测试 ====================
    // 注意：bool 实际存储为 uint8_t，true -> 1, false -> 0
    {
        std::vector<bool> vec;
        for (int i = 0; i < 100; ++i) {
            vec.push_back(rand() % 2);
        }
        sylar::ByteArray::ptr ba(new sylar::ByteArray(1));
        for (auto b : vec) {
            ba->writeFuint8(b);
        }
        ba->setPosition(0);
        for (size_t i = 0; i < vec.size(); ++i) {
            bool v = ba->readFuint8();
            SYLAR_ASSERT(v == vec[i]);
        }
        SYLAR_ASSERT(ba->getReadSize() == 0);
        SYLAR_LOG_INFO(g_logger) << "writeBool/readBool (bool) len=100 base_len=1 size=" << ba->getSize();

        ba->setPosition(0);
        SYLAR_ASSERT(ba->writeToFile("/tmp/bool_100-readBool.dat"));
        sylar::ByteArray::ptr ba2(new sylar::ByteArray(2));
        SYLAR_ASSERT(ba2->readFromFile("/tmp/bool_100-readBool.dat"));
        ba2->setPosition(0);
        SYLAR_ASSERT(ba->toString() == ba2->toString());
        SYLAR_ASSERT(ba->getPosition() == 0);
        SYLAR_ASSERT(ba2->getPosition() == 0);
    }

    // ==================== 字符串测试 ====================
    // 假设 writeString 会先写入长度（变长整型），然后写入内容
    {
        std::vector<std::string> vec;
        const char* chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        for (int i = 0; i < 100; ++i) {
            std::string str;
            int len = rand() % 50 + 1; // 1~50 长度的随机字符串
            for (int j = 0; j < len; ++j) {
                str += chars[rand() % (sizeof(chars) - 1)];
            }
            vec.push_back(str);
        }

        sylar::ByteArray::ptr ba(new sylar::ByteArray(1));
        for (auto& str : vec) {
            ba->writeStringVint(str);
        }
        ba->setPosition(0);
        for (size_t i = 0; i < vec.size(); ++i) {
            std::string v = ba->readStringVint();
            SYLAR_ASSERT(v == vec[i]);
        }
        SYLAR_ASSERT(ba->getReadSize() == 0);
        SYLAR_LOG_INFO(g_logger) << "writeString/readString (std::string) len=100 base_len=1 size=" << ba->getSize();

        ba->setPosition(0);
        SYLAR_ASSERT(ba->writeToFile("/tmp/string_100-readString.dat"));
        sylar::ByteArray::ptr ba2(new sylar::ByteArray(2));
        SYLAR_ASSERT(ba2->readFromFile("/tmp/string_100-readString.dat"));
        ba2->setPosition(0);
        SYLAR_ASSERT(ba->toString() == ba2->toString());
        SYLAR_ASSERT(ba->getPosition() == 0);
        SYLAR_ASSERT(ba2->getPosition() == 0);
    }

    // ==================== 边界值测试：大整数（尤其是变长编码）====================
    {
        // 测试变长编码对大数的支持
        std::vector<uint64_t> vec = {
            0, 1, 127, 128, 255, 256,
            (1ULL << 14) - 1, (1ULL << 14),
            (1ULL << 28) - 1, (1ULL << 28),
            (1ULL << 42) - 1, (1ULL << 42),
            (1ULL << 56) - 1, (1ULL << 56),
            UINT64_MAX
        };

        sylar::ByteArray::ptr ba(new sylar::ByteArray(1));
        for (auto val : vec) {
            ba->writeUint64(val);
        }
        ba->setPosition(0);
        for (size_t i = 0; i < vec.size(); ++i) {
            uint64_t v = ba->readUint64();
            SYLAR_ASSERT(v == vec[i]);
        }
        SYLAR_ASSERT(ba->getReadSize() == 0);
        SYLAR_LOG_INFO(g_logger) << "writeUint64/readUint64 (boundary values) count=" << vec.size() << " size=" << ba->getSize();
    }

#undef XX
#undef XX_FILE
}
int main(int argc, char** argv) {
    //test();
    testExtended();
    return 0;
}
