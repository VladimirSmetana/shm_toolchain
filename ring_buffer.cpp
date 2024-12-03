#include <algorithm>
#include <list>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include "ring_buffer.h"

RingBuffer::RingBuffer(std::string id, RING_BUF_TYPE type) : read_pos_(0),
                                                             type_(type),
                                                             size_ring_(0)
{
    char id_char[32];
    id.copy(id_char, sizeof id_char - 1);
    memcpy(id_, id_char, 32);
}
RingBuffer::~RingBuffer()
{
    detach();
}

template <typename T>
void RingBuffer::set_ptr(T *&ptr, key_t shared_id, unsigned int size, int block_index)
{
    shblock_id[block_index] = shmget(shared_id, size, 0666 | IPC_CREAT);
    if (shblock_id[block_index] < 0)
    {
        std::cerr << "shmget error" << std::endl;
        return;
    }
    ptr = static_cast<T *>(shmat(shblock_id[block_index], NULL, 0));
}

int RingBuffer::create(unsigned int size, unsigned int chunk, key_t shared_id)
{
    set_ptr(head_ptr_, shared_id, sizeof(RingBufHead), 0);
    set_ptr(data_ptr_, shared_id + 1, size, 1);
    set_ptr(checksum_ptr_, shared_id + 2, sizeof(uint8_t), 2);

    head_ptr_->rcounter = 1;
    memcpy(head_ptr_->id, id_, 32);
    head_ptr_->type = type_;
    head_ptr_->rcounter = 1;
    head_ptr_->icounter = 1;
    head_ptr_->ownid = ownID();
    head_ptr_->wflag = 0;
    head_ptr_->size = size;
    head_ptr_->length = head_ptr_->size / chunk;
    head_ptr_->chunk = chunk;
    head_ptr_->write_pos = 0;
    return 0;
}

uint32_t RingBuffer::ownID()
{
    return (reinterpret_cast<intptr_t>(this)) & 0xFFFFFFFF;
}

bool RingBuffer::isAttached() const
{
    return data_ptr_ != nullptr ? true : false;
}

int RingBuffer::attach(key_t shared_id)
{
    if (isAttached())
    {
        return -1;
    }

    set_ptr(head_ptr_, shared_id, sizeof(RingBufHead), 0);
    set_ptr(data_ptr_, shared_id + 1, head_ptr_->size, 1);
    set_ptr(checksum_ptr_, shared_id + 2, sizeof(uint8_t), 2);
    head_ptr_->rcounter++;
    return 0;
}

int RingBuffer::detach()
{
    if (!isAttached())
    {
        return -1;
    }

    head_ptr_->rcounter--;
    if (head_ptr_->ownid == ownID())
    {
        head_ptr_->oflag = 0;
    }
    shmdt(data_ptr_);
    shmdt(head_ptr_);
    shmdt(checksum_ptr_);

    for (auto id : shblock_id)
    {
        shmctl(id, IPC_RMID, nullptr);
    }

    return 0;
}

RING_BUF_TYPE RingBuffer::type() const
{
    return type_;
}
int RingBuffer::aviableReadData(unsigned int *length) const
{
    if (!isAttached())
    {
        return -1;
    }
    *length = ((head_ptr_->write_pos + head_ptr_->size) - read_pos_) % head_ptr_->length;
    return 0;
}

int RingBuffer::readStart(char **buf, unsigned int *count)
{
    if (!isAttached())
    {
        return -1;
    }

    read_pos_ = head_ptr_->write_pos;
    *buf = data_ptr_;
    *count = 0;
    return 0;
}

int RingBuffer::readStart(char *base, char **buf, unsigned int *count)
{
    if (!isAttached() ||
        base - data_ptr_ >= head_ptr_->size)
    {
        return -1;
    }

    read_pos_ = (base - data_ptr_) / head_ptr_->chunk;
    *buf = base;
    *count = 0;
    return 0;
}

int RingBuffer::readData(char **buf, unsigned int *count)
{
    if (!isAttached())
    {
        return -1;
    }

    uint32_t avialable_data = ((head_ptr_->write_pos + head_ptr_->length) - read_pos_) % head_ptr_->length;
    if ((*count) > avialable_data)
    {
        *count = avialable_data;
    }
    read_pos_ = (read_pos_ + (*count)) % head_ptr_->length;
    *buf = data_ptr_ + read_pos_ * head_ptr_->chunk;
    *count = ((head_ptr_->write_pos + head_ptr_->length) - read_pos_) % head_ptr_->length;
    return 0;
}

int RingBuffer::writeStart(char **buf, unsigned int *count)
{
    if (!isAttached())
    {
        return -1;
    }

    head_ptr_->wflag = 1;
    *buf = data_ptr_ + head_ptr_->write_pos * head_ptr_->chunk;
    *count = head_ptr_->length - head_ptr_->write_pos;
    return 0;
}

int RingBuffer::writeData(char **buf, unsigned int *count)
{
    if (!isAttached())
    {
        return -1;
    }

    if (head_ptr_->wflag == 0)
    {
        return -1;
    }
    head_ptr_->elem_size = *count;

    head_ptr_->write_pos += (*count);
    if (head_ptr_->write_pos >= head_ptr_->length)
    {
        head_ptr_->write_pos = 0;
    }
    *buf = data_ptr_ + head_ptr_->write_pos * head_ptr_->chunk;
    *count = head_ptr_->length - head_ptr_->write_pos;
    return 0;
}

const char *RingBuffer::writePos()
{
    if (!isAttached())
    {
        return nullptr;
    }

    return data_ptr_ + head_ptr_->write_pos * head_ptr_->chunk;
}

int RingBuffer::writeStop()
{
    if (!isAttached())
    {
        return -1;
    }
    head_ptr_->wflag = 0;
    return 0;
}

/**
 * @return size of buffer;
 */
uint32_t RingBuffer::size()
{
    return head_ptr_->size;
}

uint32_t RingBuffer::length()
{
    return head_ptr_->length;
}

const char *RingBuffer::data()
{
    return data_ptr_;
}

uint32_t RingBuffer::elem_size()
{
    return head_ptr_->elem_size;
}

void RingBuffer::READinit_buffer(std::shared_ptr<RingBuffer> buffer, key_t shared_id)
{
    buffer->readStart(&read_ptr, &read_size);
    int cr3 = buffer->attach(shared_id);
    if (cr3 == -1)
    {
        std::cout << "Attach error" << std::endl;
    }
}

uint8_t *RingBuffer::read_element(std::shared_ptr<RingBuffer> buffer, unsigned int elem_size)
{
    read_size = elem_size;
    buffer->readData(&read_ptr, &read_size);
    uint8_t *pfirst = reinterpret_cast<uint8_t *>(read_ptr);
    return pfirst;
};

void RingBuffer::WRITEinit_buffer(std::shared_ptr<RingBuffer> buffer, key_t shared_id, unsigned int buf_size)
{
    int cr2 = buffer->create(buf_size, 1, shared_id);
    if (cr2 == -1)
    {
        std::cout << "Creation error" << std::endl;
    }

    buffer->writeStart(&write_ptr, &write_size);
    if (write_size != buf_size)
    {
        perror("WriteError:");
    }
};

void RingBuffer::add_element(std::shared_ptr<RingBuffer> buffer, uint8_t *elem, unsigned int elem_size)
{
    write_size = elem_size;
    buffer->writeData(&write_ptr, &write_size);
    uint8_t *pfirst = reinterpret_cast<uint8_t *>(write_ptr);
    std::memcpy(pfirst, elem, elem_size);
};

void RingBuffer::WRITEuninit_buffer(std::shared_ptr<RingBuffer> buffer)
{
    buffer->writeStop();
}