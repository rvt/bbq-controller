#pragma once
#include <stdint.h>
#include <map>
#include <string.h>

class PropertyValue {
private:
    union {
        int32_t m_long;
        float m_float;
        char* m_char;
        bool m_bool;
    };

    enum Type {
        LONG,
        FLOAT,
        CHARPTR,
        BOOL
    } m_type;

public:
    explicit PropertyValue() = delete;
    explicit PropertyValue(int32_t p_long);
    explicit PropertyValue(const char* p_char);
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

    int32_t getLong() const;
    float getFloat() const;
    bool getBool() const;
    const char* getCharPtr() const;

    /**
     * Get the long value from any type
     * float: use std::lsround(..). overflows undefined behavior
     * bool: returns 1 or 0
     * char: uses atol(..)
     * long: value itself
     */
    int32_t asLong() const;

    /**
     * Get the float value from any type
     * float: use value itself
     * bool: returns 1.0f or 0.0f
     * char: uses atof(..)
     * long: standard cast
     */
    float asFloat() const;

    /**
    * Get the bool value from any type
    * float: returns false when value was 0.0f
    * bool: value itself
    * char: uses PropertyValue::boolProperty(m_char).getBool();
    * long: 0 for false
    */
    bool asBool() const;
};


class Properties {
public:

private:
    struct CompareCStrings {
        bool operator()(const char* lhs, const char* rhs) const {
            return strcmp(lhs, rhs) < 0;
        }
    };

    std::map<const char*, const PropertyValue, CompareCStrings> m_type;

public:
    void put(const char* p_entry, const PropertyValue& value);
    const PropertyValue& get(const char* p_entry) const;
};