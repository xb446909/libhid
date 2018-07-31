#pragma once

void* __stdcall open_dev(unsigned short vid, unsigned short pid);
void __stdcall close_dev(void* h);
int __stdcall write_bytes(void* h, unsigned char* bytes, int count);
int __stdcall read_bytes(void* h, unsigned char* bytes, int size);
