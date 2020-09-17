#include "propertyutils.h"

#include <memory>
#include <algorithm>

#include <cassert>
#include <cmath>
#include <stdio.h>
#include <cstring>

PropertyValue::PropertyValue(int32_t p_long) : m_long{p_long}, m_type{Type::LONG} {
}
PropertyValue::PropertyValue(const char* p_char) : m_string{p_char}, m_type{Type::STRING} {
}
PropertyValue::PropertyValue(const std::string& p_string) : m_string{p_string}, m_type{Type::STRING} {
}
PropertyValue::PropertyValue(float p_float) : m_float{p_float}, m_type{Type::FLOAT} {
}
PropertyValue::PropertyValue(bool p_bool) : m_bool{p_bool}, m_type{Type::BOOL} {
}
PropertyValue::PropertyValue() : m_type{Type::EMPTY} {
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
    if (m_type == Type::STRING) {
        m_string.~basic_string();
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

        case Type::STRING:
            new (&m_string) auto(value.m_string);
            break;

        case Type::EMPTY:
            break;
    }

    m_type = value.m_type;
}

//////////////////////////////////////////////////////////////////
// Builders
PropertyValue PropertyValue::longProperty(const char* p_char) {
    return PropertyValue((int32_t)atol(p_char));
}
PropertyValue PropertyValue::floatProperty(const char* p_char) {
    return PropertyValue((float)(std::atof(p_char)));
}
PropertyValue PropertyValue::boolProperty(const char* p_char) {
    return PropertyValue(strcmp(p_char, "true") == 0 || strcmp(p_char, "yes") == 0 || strcmp(p_char, "1") == 0);
}

//////////////////////////////////////////////////////////////////
PropertyValue::operator long() const {
    assert(m_type == Type::LONG);
    return m_long;
}
PropertyValue::operator float() const {
    assert(m_type == Type::FLOAT);
    return m_float;
}
PropertyValue::operator bool() const {
    assert(m_type == Type::BOOL);
    return m_bool;
}
PropertyValue::operator const char* () const {
    assert(m_type == Type::STRING);
    return reinterpret_cast<const char*>(m_string.c_str());
}

//////////////////////////////////////////////////////////////////

long PropertyValue::asLong() const {
    switch (m_type) {
        case Type::LONG:
            return m_long;

        case Type::FLOAT:
            return round(m_float);

        case Type::BOOL:
            return m_bool ? 1 : 0;

        case Type::STRING:
            if (m_string.length() == 0) {
                return 0L;
            }

            return round(std::atof(m_string.c_str()));
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

        case Type::STRING:
            if (m_string.length() == 0) {
                return 0.f;
            }

            return std::atof(m_string.c_str());
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

        case Type::STRING:
            if (m_string.length() == 0) {
                return false;
            }

            return (bool)PropertyValue::boolProperty(m_string.c_str());
            break;

        default:
            return false;
    }
}

PropertyValue::Type PropertyValue::type() const {
    return m_type;
}

static PropertyValue::Type charToType(char dataType) {
    switch (dataType) {
        case 'B':
            return PropertyValue::Type::BOOL;

        case 'L':
            return PropertyValue::Type::LONG;

        case 'S':
            return PropertyValue::Type::STRING;

        case 'F':
            return PropertyValue::Type::FLOAT;

        default:
            return PropertyValue::Type::EMPTY;
            break;
    }
}


//////////////////////////////////////////////////////////////////


void Properties::put(const char* p_entry, const PropertyValue& value) {
    put(std::string(p_entry), value);
}
void Properties::put(const std::string& p_entry, const PropertyValue& value) {
    auto it = m_type.find(p_entry);

    if (it != m_type.end()) {
        m_type.erase(it);
    }

    m_type.emplace(p_entry, value);
}

bool Properties::putNotContains(const std::string& p_entry, const PropertyValue& value) {
    if (!contains(p_entry)) {
        m_type.emplace(p_entry, value);
        return true;
    }

    return false;
}

bool Properties::putNotContains(const char* p_entry, const PropertyValue& value) {
    return putNotContains(std::string(p_entry), value);
}

void Properties::erase(const std::string& p_entry) {
    m_type.erase(p_entry);
}

bool Properties::contains(const std::string& p_entry) const {
    return m_type.find(p_entry) != m_type.end();
}

const PropertyValue& Properties::get(const std::string& p_entry) const {
    if (m_type.find(p_entry) != m_type.end()) {
        return m_type.at(p_entry);
    } else {
        static auto pv = PropertyValue();
        return pv;
    }
}

// https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
char* Properties::stripWS_LT(char* str) {
    char* end;

    // Trim leading space
    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == 0) { // All spaces?
        return str;
    }

    // Trim trailing space
    end = str + strlen(str) - 1;

    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

char* Properties::getNextNonSpaceChar(char* buffer) {
    while (*buffer != 0 && isspace(*buffer)) {
        buffer++;
    }

    return buffer;
}

void Properties::serializeProperties(char* v, size_t desiredCapacity, Stream& device) {
    for (auto it = m_type.begin(); it != m_type.end(); ++it) {

        switch (it->second.m_type) {
            case PropertyValue::Type::LONG:
                snprintf(v, desiredCapacity, "%s=%c%ld", it->first.c_str(), 'L', (long)it->second);
                break;

            case PropertyValue::Type::FLOAT:
                snprintf(v, desiredCapacity, "%s=%c%g", it->first.c_str(), 'F', (float)it->second);
                break;

            case PropertyValue::Type::BOOL:
                snprintf(v, desiredCapacity, "%s=%c%i", it->first.c_str(), 'B', (bool)it->second ? 1 : 0);
                break;

            case PropertyValue::Type::STRING:
                snprintf(v, desiredCapacity, "%s=%c%s", it->first.c_str(), 'S', (const char*)it->second);
                break;

            case PropertyValue::Type::EMPTY:
                // Not found propery type
                break;
        }

        device.print(v);
        device.print("\n");
    }
}

void Properties::deserializeProperties(char* buffer, size_t desiredCapacity, Stream& device) {
    while (device.available()) {
        uint16_t readBytes = device.readBytesUntil('\n', buffer, desiredCapacity - 1);
        buffer[readBytes] = 0;

        if (readBytes > 0) {
            char* buffPtr = strchr(buffer, '=');

            if (buffPtr != nullptr) {
                *buffPtr = 0; // null terminate after variable name
                buffPtr = getNextNonSpaceChar(++buffPtr);
                // Data type of this variable
                PropertyValue::Type dataType = charToType(*buffPtr);
                // trim variable name and variable
                char* variableName = stripWS_LT(buffer);
                char* variableValue = stripWS_LT(++buffPtr);

                // If the entry already exists, we must ensure type is the same
                PropertyValue::Type currentType = get(variableName).type();

                if (currentType != PropertyValue::Type::EMPTY) {
                    dataType = currentType;
                }

                switch (dataType) {
                    case PropertyValue::Type::BOOL:
                        put(variableName, PropertyValue::boolProperty(variableValue));
                        break;

                    case PropertyValue::Type::LONG:
                        put(variableName, PropertyValue::longProperty(variableValue));
                        break;

                    case PropertyValue::Type::STRING:
                        put(variableName, PropertyValue(variableValue));
                        break;

                    case PropertyValue::Type::FLOAT:
                        put(variableName, PropertyValue::floatProperty(variableValue));
                        break;

                    default:
                        break;
                        // std::cout << ":" << variableName << ":" << dataType << ":" << variableValue << "\n";
                }
            }
        }

        // Read untill we find the next non-space character
        while (isspace(device.peek()) != 0) {
            device.read();
        }
    }
}