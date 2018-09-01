//
// Created by ProDigital on 8/2/18.
//

#ifndef GPPROTO_OUTPUTSTREAMEXCEPTION_H
#define GPPROTO_OUTPUTSTREAMEXCEPTION_H

#include <exception>
#include <string>

namespace gpproto {
    class InputStreamException : public std::exception {
    public:
        const std::string message;
        const int code;

        InputStreamException(std::string message, int code) : message(std::move(message)), code(code) {

        }
    };
}
#endif //GPPROTO_OUTPUTSTREAMEXCEPTION_H
