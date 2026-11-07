#pragma once

struct Message;

//Drivers post into the kernel->WM ring published under the "input" endpoint, which the usermode WM drains
void KeInputInitialize();
void KeInputPost(const Message& message);
