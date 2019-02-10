#pragma once

#include <openssl/sha.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>

namespace hashing
{

unsigned long getSizeByFileDescriptor(int fd)
{
    struct stat statbuf;
    if (fstat(fd, &statbuf) < 0)
        exit(-1);
    return statbuf.st_size;
}

std::string getFileHash(const char *filePath)
{
    unsigned char hashResult[SHA512_DIGEST_LENGTH];

    // Read file
    int fileDescriptor{open(filePath, O_RDONLY)};
    unsigned long fileSize{getSizeByFileDescriptor(fileDescriptor)};

    // Allocate memory, get hash, release
    void *fileBuffer{mmap(0, fileSize, PROT_READ,
                          MAP_SHARED, fileDescriptor, 0)};
    SHA512((unsigned char *)fileBuffer, fileSize, hashResult);
    munmap(fileBuffer, fileSize);

    // turn into readable string
    std::string result{};
    char buffer[3];
    for (int i = 0; i < SHA512_DIGEST_LENGTH; i++)
    {
        sprintf(buffer, "%02x", hashResult[i]);
        result += buffer;
    }

    return result;
}
} // namespace hashing
