#include "propertyutils.h"

#include <stdlib.h>
#include <memory>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <string>
#include <cstdlib>


PropertyValue::PropertyValue(int32_t p_long) : m_long(p_long), m_type(Type::LONG) {
}
PropertyValue::PropertyValue(const char* p_char) : m_char(strdup(p_char)), m_type(Type::CHARPTR) {
}
PropertyValue::PropertyValue(float p_float) : m_float(p_float), m_type(Type::FLOAT) {
}
PropertyValue::PropertyValue(bool p_bool) : m_bool(p_bool), m_type(Type::BOOL) {
}

PropertyValue::~PropertyValue() {
    destroy();
}

PropertyValue::PropertyValue(const PropertyValue& val) {
    copy(val);
}

PropertyValue& PropertyValue::operator=(const PropertyValue& val) {
    // Nothing to do in case of self-assignment
    if (&val != this) {
        destroy();
        copy(val);
    }

    return *this;
}

void PropertyValue::destroy() {
    if (m_type == Type::CHARPTR) {
        delete m_char;
    }
}

void PropertyValue::copy(const PropertyValue& value) {
    switch (value.m_type) {
        case Type::LONG:
            m_long = value.m_long;
            break;

        case Type::FLOAT:
            m_float = value.m_float;
            break;

        case Type::BOOL:
            m_bool = value.m_bool;
            break;

        case Type::CHARPTR:
            m_char = strdup(value.m_char);
            break;
    }

    m_type = value.m_type;
}

PropertyValue PropertyValue::longProperty(const char* p_char) {
    return PropertyValue((int32_t)atol(p_char));
}
PropertyValue PropertyValue::floatProperty(const char* p_char) {
    return PropertyValue((float)(std::atof(p_char)));
}
PropertyValue PropertyValue::boolProperty(const char* p_char) {
    return PropertyValue(strcmp(p_char, "true") == 0 || strcmp(p_char, "yes") == 0 || strcmp(p_char, "1") == 0);
}

int32_t PropertyValue::getLong() const {
    assert(m_type == Type::LONG);
    return m_long;
}
float PropertyValue::getFloat() const {
    assert(m_type == Type::FLOAT);
    return m_float;
}
bool PropertyValue::getBool() const {
    assert(m_type == Type::BOOL);
    return m_bool;
}
const char* PropertyValue::getCharPtr() const {
    assert(m_type == Type::CHARPTR);
    return m_char;
}

int32_t PropertyValue::asLong() const {
    switch (m_type) {
        case Type::LONG:
            return m_long;

        case Type::FLOAT:
            return round(m_float);

        case Type::BOOL:
            return m_bool ? 1 : 0;

        case Type::CHARPTR:
            return round(std::atof(m_char));
            break;

        default:
            return 0;
    }
}

float PropertyValue::asFloat() const {
    switch (m_type) {
        case Type::LONG:
            return m_long;

        case Type::FLOAT:
            return m_float;

        case Type::BOOL:
            return m_bool ? 1.0f : 0.0f;

        case Type::CHARPTR:
            return std::atof(m_char);
            break;

        default:
            return 0.0f;
    }
}

bool PropertyValue::asBool() const {
    switch (m_type) {
        case Type::LONG:
            return m_long != 0;

        case Type::FLOAT:
            return m_float != 0.0f;

        case Type::BOOL:
            return m_bool;

        case Type::CHARPTR:
            return PropertyValue::boolProperty(m_char).getBool();
            break;

        default:
            return false;
    }
}

void Properties::put(const char* p_entry, const PropertyValue& value) {
    m_type.emplace(p_entry, value);
}

const PropertyValue& Properties::get(const char* p_entry) const {
    return m_type.at(p_entry);
}
