#include "byte_stream.hh"

ByteStream::ByteStream(const size_t capacity) : _capacity(capacity), _wr_count(0), _rd_count(0) {}

/* "Input" interface for the writer */

size_t ByteStream::remaining_capacity() const { return _capacity - _buffer.size(); }

void ByteStream::end_input() {
    _ieof = true;
    return;
}

size_t ByteStream::write(const std::string &data) {
    size_t size = std::min(data.size(), remaining_capacity());
    _buffer.append(BufferList(data.substr(0, size)));
    _wr_count += size;
    return size;
}

/* "Output" interface for the reader */

bool ByteStream::input_ended() const { return _ieof; }

size_t ByteStream::buffer_size() const { return _buffer.size(); }

bool ByteStream::buffer_empty() const { return _buffer.size() == 0; }

bool ByteStream::eof() const { return _ieof && _buffer.size() == 0; }

std::string ByteStream::peek_output(const size_t len) const {
    size_t size = std::min(len, _buffer.size());
    return _buffer.concatenate().substr(0, size);
}

void ByteStream::pop_output(const size_t len) {
    int size = std::min(len, _buffer.size());
    _rd_count += size;
    _buffer.remove_prefix(size);
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
