//
// Created by ProDigital on 8/2/18.
//

#ifndef GPPROTO_OUTPUTSTREAMEXCEPTION_H
#define GPPROTO_OUTPUTSTREAMEXCEPTION_H

#include <exception>
#include <string>

namespace gpproto {
    class OutputStreamException : public std::exception {
    public:
        const std::string message;
        const int code;

        OutputStreamException(const std::string &message, int code) : message(message), code(code) {

        }
    };
}
#endif //GPPROTO_OUTPUTSTREAMEXCEPTION_H
