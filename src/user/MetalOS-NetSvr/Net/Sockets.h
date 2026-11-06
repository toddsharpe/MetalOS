#pragma once

#include "Net/Net.h"
#include "Net/KSocket.h"

//The stack's socket layer (Sockets.cpp), which Main.cpp drives on behalf of apps over IPC
KSocket* SocketCreate(const int type, const int proto);
void SocketBind(KSocket& socket, const Net::endpoint_t& local);
bool SocketSend(KSocket& socket, const Net::endpoint_t& dst, const void* const buffer, const size_t length);
bool SocketRecv(KSocket& socket, Net::endpoint_t& src, void* const buffer, const size_t maxLength, size_t& bytesRead);
void SocketClose(KSocket& socket);
