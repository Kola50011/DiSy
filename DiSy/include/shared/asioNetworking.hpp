#pragma once

#include <google/protobuf/message.h>
#include <asio.hpp>
#include <unordered_map>
#include <typeindex>

#include "DiSy.pb.h"
#include "DiSy.grpc.pb.h"

using namespace asio::ip;
using asio::buffer;
using asio::error_code;
using asio::streambuf;
using asio::write;
using asio::ip::tcp;

using std::istream;
using std::ostream;

// Extracts integral value of enum class variants
template <typename E>
constexpr auto toUnderlying(E e) noexcept
{
    return static_cast<u_int8_t>(static_cast<std::underlying_type_t<E>>(e));
}

namespace networking
{
const int SEND_OK{0};
const int SEND_ERR{1};

enum class MessageType
{
    FileMetadata = 1,
    File = 2
};

const std::unordered_map<std::type_index, MessageType> typeMapping{
    {typeid(DiSy::FileMetadata), MessageType::FileMetadata},
    {typeid(DiSy::File), MessageType::File}};

int sendProto(tcp::socket &socket, google::protobuf::Message &message)
{
    error_code errorCode;

    u_int8_t messageType{toUnderlying(typeMapping.at(typeid(message)))};
    u_int64_t messageSize{message.ByteSizeLong()};

    asio::write(socket, buffer(&messageType, sizeof(messageType)), errorCode);
    if (errorCode)
    {
        return SEND_ERR;
    }

    asio::write(socket, buffer(&messageSize, sizeof(messageSize)), errorCode);
    if (errorCode)
    {
        return SEND_ERR;
    }

    streambuf streamBuffer;
    ostream outputStream(&streamBuffer);
    message.SerializeToOstream(&outputStream);

    asio::write(socket, streamBuffer, errorCode);
    if (errorCode)
    {
        return SEND_ERR;
    }
    return SEND_OK;
}

int receiveProtoMessageType(tcp::socket &socket, MessageType &messageType)
{
    error_code errorCode;
    u_int8_t messageTypeRaw;

    socket.receive(buffer(&messageTypeRaw, sizeof(messageTypeRaw)), 0, errorCode);
    if (errorCode)
    {
        return SEND_ERR;
    }
    messageType = static_cast<MessageType>(messageTypeRaw);

    return SEND_OK;
}

int receiveProtoMessage(tcp::socket &socket, google::protobuf::Message &message)
{
    error_code errorCode;
    u_int64_t messageSize;

    socket.receive(buffer(&messageSize, sizeof(messageSize)), 0, errorCode);
    if (errorCode)
    {
        return SEND_ERR;
    }

    streambuf streamBuffer;

    streambuf::mutable_buffers_type mutableBuffer{streamBuffer.prepare(messageSize)};

    streamBuffer.commit(read(socket, mutableBuffer, errorCode));
    if (errorCode)
    {
        return SEND_ERR;
    }

    istream inputStream{&streamBuffer};
    message.ParseFromIstream(&inputStream);

    return SEND_OK;
}
} // namespace networking