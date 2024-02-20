#pragma once
//#include <memory>
#include <string>

typedef float  float_t;
typedef double double_t;
//typedef string String;

# ifndef NAN
#  define NAN (__builtin_nanf(""))
# endif

typedef enum {
    PT_I8, PT_U8, PT_I16, PT_U16, PT_I32, PT_U32, PT_I64, PT_U64, PT_STR, PT_BLOB, PT_INVALID
} PreferenceType;

class Preferences {
    public:
        Preferences() {};
        ~Preferences() {};

        bool begin(const char * name, bool readOnly=false, const char* partition_label=NULL) {return true;};
        void end() {};

        bool clear() {return true;};
        bool remove(const char * key) {return true;};

        size_t putChar(const char* key, int8_t value) {return putUChar(key, (uint8_t) (value ? 1 : 0));};
        size_t putUChar(const char* key, uint8_t value) {return 1;};
        size_t putShort(const char* key, int16_t value) {return 1;};
        size_t putUShort(const char* key, uint16_t value) {return 1;};
        size_t putInt(const char* key, int32_t value) {return 1;};
        size_t putUInt(const char* key, uint32_t value) {return 1;};
        size_t putLong(const char* key, int32_t value) {return putInt(key, value);};
        size_t putULong(const char* key, uint32_t value) {return putUInt(key, value);};
        size_t putLong64(const char* key, int64_t value) {return 1;};
        size_t putULong64(const char* key, uint64_t value) {return 1;};
        size_t putFloat(const char* key, float_t value) {return putBytes(key, (void*)&value, sizeof(float_t));};
        size_t putDouble(const char* key, double_t value) {return putBytes(key, (void*)&value, sizeof(double_t));};
        size_t putBool(const char* key, bool value) {return putUChar(key, (uint8_t) (value ? 1 : 0));};
        size_t putString(const char* key, const char* value) {return 1;};
        size_t putString(const char* key, std::string value) {return putString(key, value.c_str());};
        size_t putBytes(const char* key, const void* value, size_t len) {return 1;};

        bool isKey(const char* key) {return true;};
        PreferenceType getType(const char* key) {return PT_INVALID;};
        int8_t getChar(const char* key, int8_t defaultValue = 0) {return defaultValue;};
        uint8_t getUChar(const char* key, uint8_t defaultValue = 0) {return defaultValue;};
        int16_t getShort(const char* key, int16_t defaultValue = 0) {return defaultValue;};
        uint16_t getUShort(const char* key, uint16_t defaultValue = 0) {return defaultValue;};
        int32_t getInt(const char* key, int32_t defaultValue = 0) {return defaultValue;};
        uint32_t getUInt(const char* key, uint32_t defaultValue = 0) {return defaultValue;};
        int32_t getLong(const char* key, int32_t defaultValue = 0) {return defaultValue;};
        uint32_t getULong(const char* key, uint32_t defaultValue = 0) {return defaultValue;};
        int64_t getLong64(const char* key, int64_t defaultValue = 0) {return defaultValue;};
        uint64_t getULong64(const char* key, uint64_t defaultValue = 0) {return defaultValue;};
        float_t getFloat(const char* key, float_t defaultValue = NAN) {return defaultValue;};
        double_t getDouble(const char* key, double_t defaultValue = NAN) {return defaultValue;};
        bool getBool(const char* key, bool defaultValue = false) {return defaultValue;};
        size_t getString(const char* key, char* value, size_t maxLen) {return maxLen;};
        std::string getString(const char* key, std::string defaultValue) {return defaultValue;};
        size_t getBytesLength(const char* key) {return 0;};
        size_t getBytes(const char* key, void * buf, size_t maxLen)  {return 0;};
        size_t freeEntries()  {return 0;};
};
