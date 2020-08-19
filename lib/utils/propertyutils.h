#pragma once
#include <string>
#include <stdint.h>
#include <map>
#include <Stream.h>

class Properties;

class PropertyValue {
private:
    union {
        int32_t m_long;
        float m_float;
        std::string m_string;
        bool m_bool;
    };

    enum Type {
        LONG,
        FLOAT,
        STRING,
        BOOL
    } m_type;
    friend class Properties;

public:
    friend void serializeProperties(char* v, std::size_t desiredCapacity, Stream& device, const Properties& p);

    explicit PropertyValue() = delete;
    explicit PropertyValue(int32_t p_long);
    explicit PropertyValue(const char* p_char);
    explicit PropertyValue(const std::string& p_string);
    explicit PropertyValue(float p_float);
    explicit PropertyValue(bool p_bool);

    virtual ~PropertyValue();

    PropertyValue(const PropertyValue& val);
    PropertyValue& operator=(const PropertyValue& val);

    void copy(const PropertyValue&);
    void destroy();


    // Create a long properety from string
    static PropertyValue longProperty(const char* p_char);
    // Create a float property from string
    static PropertyValue floatProperty(const char* p_char);
    // Create a bool property from string
    static PropertyValue boolProperty(const char* p_char);

    bool asBool() const;
    float asFloat() const;
    long asLong() const;

    //////////////////////////////////////////////////////////////////


    operator const char* () const;

    /**
     * Get the float value from any type
     * float: use value itself
     * bool: returns 1.0f or 0.0f
     * char: uses atof(..)
     * long: standard cast
     */
    operator float() const;

    /**
    * Get the bool value from any type
    * float: returns false when value was 0.0f
    * bool: value itself
    * char: uses PropertyValue::boolProperty(m_char).getBool();
    * long: 0 for false
    */
    operator bool() const;

    /**
     * Get the long value from any type
     * float: use std::lsround(..). overflows undefined behavior
     * bool: returns 1 or 0
     * char: uses atol(..)
     * long: value itself
     */
    operator long() const;
    operator int16_t() const {
        return (long) * this;
    }
    operator int32_t() const {
        return (long) * this;
    }
    operator char() const {
        return (long) * this;
    }

};

class Properties {
private:

    std::map<std::string, const PropertyValue> m_type;

public:
    template<std::size_t desiredCapacity>
    friend void serializeProperties(Stream& device, Properties& properties);
    template<std::size_t desiredCapacity>
    friend void deserializeProperties(Stream& device, Properties& properties);

    void erase(const std::string& p_entry);

    void put(const std::string& p_entry, const PropertyValue& value);
    void put(const char* p_entry, const PropertyValue& value);
    bool putNotContains(const std::string& p_entry, const PropertyValue& value);
    bool putNotContains(const char* p_entry, const PropertyValue& value);
    const PropertyValue& get(const std::string& p_entry) const;
    bool contains(const std::string& p_entry) const;

private:
    char* stripWS_LT(char* str);
    char* getNextNonSpaceChar(char* buffer);
    void serializeProperties(char* v, size_t desiredCapacity, Stream& device);
    void deserializeProperties(char* buffer, size_t desiredCapacity, Stream& device);
};

template<std::size_t desiredCapacity>
void serializeProperties(Stream& device, Properties& p) {
    static_assert(desiredCapacity > 0, "Must be > 0");
    char buffer[desiredCapacity];
    p.serializeProperties(buffer, desiredCapacity, device);
}

template<std::size_t desiredCapacity>
void deserializeProperties(Stream& device, Properties& p) {
    static_assert(desiredCapacity > 0, "Must be > 0");
    char buffer[desiredCapacity];
    p.deserializeProperties(buffer, desiredCapacity, device);
}