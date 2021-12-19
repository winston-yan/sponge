#include "stream_reassembler.hh"

#include <algorithm>

StreamReassembler::StreamReassembler(const size_t capacity) : _unasm_idx(0), _unasm_count(0), _eof_idx(static_cast<size_t>(-1)), _output(capacity), _capacity(capacity), _fragments() {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const std::string &data, const size_t index, const bool eof) {
    // process eof index
    if (eof) _eof_idx = index + data.size();

    size_t len_data = data.size(), i_data = 0;
    // to make sure data has new bytes to assemble
    if (len_data == 0) {
        if (index == _eof_idx) _output.end_input(); 
        return ;
    }
    if (index < _unasm_idx) { 
        if (index + len_data <= _unasm_idx) return ;
        i_data = _unasm_idx - index;
        len_data -= i_data;
    }

    // to avoid buffer overflow
    size_t end_idx_allowed = _unasm_idx + _capacity - _output.buffer_size();
    len_data = std::min(len_data, end_idx_allowed - (i_data + index));

    if (index <= _unasm_idx) { // write into stream: data is prior to fragments
        _output.write(data.substr(i_data, len_data));
        _unasm_idx += len_data;

        auto it = _fragments.begin();
        while (it != _fragments.end() && 
               it->first + it->second.size() <= _unasm_idx) {
            _unasm_count -= it->second.size();
            it = _fragments.erase(it);
        }

        if (it != _fragments.end() && it->first <= _unasm_idx) {
            size_t start = it->first, len = it->second.size();
            _output.write(it->second.substr(_unasm_idx - start));
            _unasm_count -= len;
            _unasm_idx = start + len;
            _fragments.erase(it);
        }

    } else { // filling gaps and coalescing fragments
        // find the first fragment that right bound is greater than or equal to index
        auto it = std::lower_bound(
                _fragments.begin(), _fragments.end(),
                std::pair<int, std::string>(index, {}),
                [] (const std::pair<int, std::string> &a, 
                    const std::pair<int, std::string> &b) {
                    return a.first + a.second.size() < b.first + b.second.size();
                });

        if (it == _fragments.end()) { // add fragment at the end of fragments
            _fragments[index] = data.substr(0, len_data);
            _unasm_count += len_data;
        } else if (it->first == index + len_data) { // coalesce (it) at right side
            _fragments[index] = data.substr(0, len_data) + it->second;
            _fragments.erase(it);
            _unasm_count += len_data;
        } else { // coalesce (it) at left side
            size_t end_idx = index + len_data;
            size_t last_right_bound = it->first + it->second.size(); 
            auto node = it;
            // cut the redundant tail of (it)
            node->second = node->second.substr(index - node->first) +
                           data.substr(0, len_data);
            
            ++it;
            while (it != _fragments.end() && it->first + it->second.size() < end_idx) {
                _unasm_count += it->first - last_right_bound;
                last_right_bound = it->first + it->second.size();
                it = _fragments.erase(it);
            }

            if (it != _fragments.end()) {
                if (end_idx < it->first) { // gap between last chunk and data
                    _unasm_count += end_idx - last_right_bound;
                } else { // coalesce (it) at right side
                    _unasm_count += it->first - last_right_bound;
                    node->second += it->second.substr(end_idx - it->first);
                    _fragments.erase(it);
                }
            }
        }
    }

    if (_eof_idx == _unasm_idx) _output.end_input();
    return ;
}

size_t StreamReassembler::unassembled_bytes() const { return this->_unasm_count; }

bool StreamReassembler::empty() const { return this->_unasm_count == 0; }
