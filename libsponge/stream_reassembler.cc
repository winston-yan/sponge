#include "stream_reassembler.hh"

#include <algorithm>
#include <iostream>

StreamReassembler::StreamReassembler(const size_t capacity)
    : _unasm_idx(0)
    , _unasm_count(0)
    , _eof_idx(static_cast<size_t>(-1))
    , _capacity(capacity)
    , _output(capacity)
    , _fragments() {}

void StreamReassembler::check_eof() {
    if (_unasm_idx == _eof_idx)
        _output.end_input();
}

size_t StreamReassembler::get_receive_base() const { return _unasm_idx; }

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const std::string &data, const size_t index, const bool eof) {
    // process eof index
    if (eof)
        _eof_idx = index + data.size();

    size_t len_data = data.size(), start_ptr = 0;
    // to make sure data has new bytes to assemble
    if (len_data == 0) {
        // corner case 1
        check_eof();
        return;
    }
    // corner case 2
    if (index + len_data <= _unasm_idx)
        return;

    // to avoid buffer overflow
    size_t end_idx_allowed = _unasm_idx + _capacity - _output.buffer_size();
    len_data = std::min(len_data, end_idx_allowed - (start_ptr + index));

    // write into stream: data is prior to fragments
    if (index <= _unasm_idx) {
        start_ptr = _unasm_idx - index;
        len_data -= start_ptr;

        _output.write(data.substr(start_ptr, len_data));
        _unasm_idx += len_data;

        auto it = _fragments.begin();
        while (it != _fragments.end() && it->end_idx <= _unasm_idx) {
            _unasm_count -= it->block.size();
            it = _fragments.erase(it);
        }

        if (it != _fragments.end() && it->begin_idx <= _unasm_idx) {
            _output.write(it->block.substr(_unasm_idx - it->begin_idx));
            _unasm_count -= it->block.size();
            _unasm_idx = it->end_idx;
            _fragments.erase(it);
        }

        check_eof();
        return;
    }

    // filling gaps and coalescing fragments
    // find the first fragment that right bound is greater than or equal to index
    auto it = std::lower_bound(_fragments.begin(),
                               _fragments.end(),
                               Node({index, index, {}}),
                               [](const Node &a, const Node &b) { return a.end_idx < b.end_idx; });

    if (it == _fragments.end()) {
        /* add data as fragment at the end of fragments */
        _fragments.insert(Node({index, index + len_data, data.substr(0, len_data)}));
        _unasm_count += len_data;
        check_eof();
        return;
    }

    size_t end_idx = index + len_data;
    size_t last_right_bound = it->end_idx;

    if (end_idx < it->begin_idx) {
        /* add data as fragment at the beginning of fragments */
        _fragments.insert(Node({index, index + len_data, data.substr(0, len_data)}));
        _unasm_count += len_data;
    } else if (end_idx == it->begin_idx) {
        /* coalesce [it] at right side */
        _fragments.insert(Node({index, end_idx + it->block.size(), data.substr(0, len_data) + it->block}));
        _fragments.erase(it);
        _unasm_count += len_data;
    } else if (index < it->begin_idx || it->end_idx < end_idx) {
        /* data overlaps with or attach to BUT NOT substring of [it] */
        auto node = Node({std::min(index, it->begin_idx), {}, {}});

        if (index < it->begin_idx) {
            /* left side of [it] falls in the range of data */
            node.block = data.substr(0, len_data);
            node.end_idx = end_idx;
            _unasm_count += it->begin_idx - index;
        } else {
            /* coalesce: left side of [it] <= left side of data */
            node.block = it->block.substr(0, index - it->begin_idx) + data.substr(0, len_data);
            node.end_idx = end_idx;
        }

        if (end_idx < it->end_idx) {
            node.block += it->block.substr(end_idx - it->begin_idx);
            node.end_idx = it->end_idx;
        }

        it = _fragments.erase(it);
        while (it != _fragments.end() && it->end_idx < end_idx) {
            _unasm_count += it->begin_idx - last_right_bound;
            last_right_bound = it->end_idx;
            it = _fragments.erase(it);
        }

        if (it != _fragments.end() && it->begin_idx <= end_idx) {
            /* coalesce [it] at right side */
            _unasm_count += it->begin_idx - last_right_bound;
            node.block += it->block.substr(end_idx - it->begin_idx);
            node.end_idx = it->end_idx;
            _fragments.erase(it);
        } else {
            /* tail part of data in the gap should be counted */
            _unasm_count += end_idx - last_right_bound;
        }

        _fragments.insert(node);
    }

    check_eof();
    return;
}

size_t StreamReassembler::unassembled_bytes() const { return this->_unasm_count; }

bool StreamReassembler::empty() const { return this->_unasm_count == 0; }
