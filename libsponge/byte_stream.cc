#include "byte_stream.hh"

ByteStream::ByteStream(const size_t capacity) : _cap(capacity), _wr_count(0), _rd_count(0) {}

/* "Input" interface for the writer */

size_t ByteStream::remaining_capacity() const { return _cap - buffer_size(); }

void ByteStream::end_input() {
    this->_ieof = true;
    return;
}

size_t ByteStream::write(const std::string &data) {
    size_t len = data.size(), rem = remaining_capacity();
    if (len > rem) {
        len = rem;
    }
    for (size_t i = 0; i < len; ++i)
        _buff.push_back(data[i]);
    _wr_count += len;
    return len;
}

/* "Output" interface for the reader */

bool ByteStream::input_ended() const { return this->_ieof; }

size_t ByteStream::buffer_size() const { return this->_buff.size(); }

bool ByteStream::buffer_empty() const { return buffer_size() == 0; }

bool ByteStream::eof() const { return this->_ieof && buffer_empty(); }

std::string ByteStream::peek_output(const size_t len) const {
    size_t rd_len = len, size = buffer_size();
    if (rd_len > size) {
        rd_len = size;
    }
    return std::string(_buff.begin(), _buff.begin() + rd_len);
}

void ByteStream::pop_output(const size_t len) {
    int size = std::min(len, buffer_size());
    _rd_count += size;
    while (size--) {
        _buff.pop_front();
    }
    return;
}

std::string ByteStream::read(const size_t len) {
    std::string sread = peek_output(len);
    pop_output(len);
    return sread;
}

/* General accounting */

size_t ByteStream::bytes_written() const { return this->_wr_count; }

size_t ByteStream::bytes_read() const { return this->_rd_count; }
