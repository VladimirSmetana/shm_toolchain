#ifndef RING_BUFFER_H
#define RING_BUFFER_H
#include <string>
#include <stdint.h>
#include <string>
#include <mutex>
#include <memory>
#include <vector>
#include "mutex.h"

#define MAX_ELEM_SIZE 6998444
enum RING_BUF_TYPE
{
    RAW = 0,
    CHUNK
};

struct RingBufHead
{
    char id[32];        // buffer name
    RING_BUF_TYPE type; // buffer name
    uint8_t oflag;      // open flag
    int32_t rcounter;   // amount of data readers
    uint32_t icounter;  // amount of init
    uint32_t ownid;     // owner id
    uint8_t wflag;      // write flag
    uint32_t size;      // full size of buffer in bytes (without header)
    uint32_t chunk;     // size of chunk data
    uint32_t length;    // full size of buffer in chunks
    uint32_t write_pos; // write position
    uint32_t elem_size; // size of element
};

class RingBuffer
{
public:
    explicit RingBuffer(std::string id, RING_BUF_TYPE type);
    RingBuffer() {}
    virtual ~RingBuffer();
    /**
     * Create buffer
     * @param[in] size size of buffer
     * @return operation status
     */
    virtual int create(unsigned int size, unsigned int chunk, key_t shared_id);

    /**
     * Attach buffer
     * @return operation status
     */
    int attach(key_t shared_id);
    /**
     * Attach buffer
     * @return operation status
     */
    int attach_to_shm(key_t key);
    /**
     * Detach buffer
     * @return operation status
     */
    int detach();

    /**
     * Check attached state
     * @return operation status
     */
    bool isAttached() const;

    /**
     * Name buffer
     * @return buffer name
     */
    const char *id() const;

    /**
     * Type buffer
     * @return buffer type
     */
    RING_BUF_TYPE type() const;

    /**
     * Get read avialbale data
     * @param[size] data_ptr link to data return value
     * @return operation status
     */
    int aviableReadData(unsigned int *length) const;

    /**
     * Start read data from ring buffer
     * @param[out] buf link to data return value
     * @param[out] count of readed data in chunks
     * @return operation status
     */
    int readStart(char **buf, unsigned int *count);
    /**
     * Start read data from ring buffer
     * @param[in] buf link to data return value
     * @param[out] buf link to data return value
     * @param[out] count of readed data in chunks
     * @return operation status
     */
    int readStart(char *base, char **buf, unsigned int *count);

    /**
     * Read data from ring buffer
     * @param[out] buf link to data return value
     * @param[in,out] count requested amount of data, amount of readed data
     * @return operation status
     */
    int readData(char **buf, unsigned int *count);

    /**
     * Start write to ring buffer
     * @param[out] buf link to data write
     * @param[out] count max write data size
     * @return operation status
     */
    int writeStart(char **buf, unsigned int *count);

    /**
     * Write data to ring buffer
     * @param[in] data_ptr double link to data write
     * @param[in,out] size writing data, space for write data
     * @return operation status
     */
    virtual int writeData(char **buf, unsigned int *count);

    /**
     * Get pointer to write position
     * @return pointer to write position
     */

    const char *writePos();

    /**
     * Finish write data
     * @return operation status
     */
    int writeStop();

    /**
     * @return size of buffer
     */
    uint32_t size();

    /**
     * @return possible amount of elements in buffer
     */
    uint32_t length();
    /**
     * @return size of element
     */
    uint32_t elem_size();
    /**
     * @return pointer to the begining of ring buffer
     */
    const char *data();

    std::timed_mutex *set_mutex_ptr();

    template <typename T>
    void set_ptr(T *&ptr, key_t shared_id, unsigned int size, int block_index);
    template <typename T>
    void get_ptr(T *&ptr, key_t shared_id, unsigned int size, int block_index);

    void WRITEinit_buffer(std::shared_ptr<RingBuffer> buffer, key_t shared_id, unsigned int buf_size);
    void add_element(std::shared_ptr<RingBuffer> buffer, uint8_t *elem, unsigned int elem_size);
    void WRITEuninit_buffer(std::shared_ptr<RingBuffer> buffer);

    void READinit_buffer(std::shared_ptr<RingBuffer> buffer, key_t shared_id);
    uint8_t *read_element(std::shared_ptr<RingBuffer> buffer, unsigned int elem_size);

    char *write_ptr;
    unsigned int write_size;

    char *read_ptr;
    unsigned int read_size;
    unsigned int *size_ptr_;

    uint8_t calculate_checksum(uint8_t *data, size_t size);
    void write_checksum(uint8_t *image_data, size_t size);
    bool validate_image(uint8_t *image_data, size_t size);

    uint32_t user_crc32(uint32_t crc, const uint8_t *buf, size_t len);
    void generate_crc32_table();
    static uint32_t u_table[256];
    uint8_t *checksum_ptr_;
    uint32_t fletcher32(const uint8_t *data, size_t len);
    uint8_t compute_signature(uint8_t *data, size_t length);

    void writer_to_reader();
    void reader_to_writer();

protected:
    uint32_t ownID();
    int shblock_id[4];
    void addHead(std::shared_ptr<RingBufHead> head);
    uint32_t read_pos_;
    char id_[32];
    RING_BUF_TYPE type_;
    char *data_ptr_;
    unsigned int size_ring_;
    RingBufHead *head_ptr_;
    RingSync *rings_;
};

#endif // RING_BUFFER_H