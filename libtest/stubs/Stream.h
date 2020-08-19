#pragma once
#include <iostream>
class Stream {
    std::string m_allIn;
    std::string m_allOut;
public:
    Stream() : m_allIn(""), m_allOut("") {
    }
    Stream(std::string out) : m_allIn(""), m_allOut(out)  {
    }
    template<typename T>
    void print(T f) {
        m_allIn = m_allIn + f;
        std::cout << f;
    }
    template<typename T>
    void println(T f) {
        m_allIn = m_allIn + f + "\n";
        std::cout << f << "\n";
    }

    std::string streamedOut() const {
        return m_allIn;
    }

    int available() {
        return m_allOut.length();
    }

    size_t readBytesUntil(char terminator, char* buffer, size_t length) {
        size_t size = 0;

        while (true) {
            if (size >= length) {
                return size;
            }

            if (!available()) {

                return size;
            }

            if (peek() == terminator) {
                return size;
            }

            buffer[size++] = read();
        }
    }

    int peek() {
        return m_allOut[0];
    }

    int read() {
        if (m_allOut.size() > 0) {
            int ans = m_allOut[0];
            m_allOut.erase(0, 1);
            return ans;
        }

        return -1;
    }


};