#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    std::cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
              << ip_address.ip() << std::endl;
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();

    EthernetFrame frame;
    EthernetHeader &header = frame.header();

    if (_table.find(next_hop_ip) != _table.end()) {
        /* If the destination Ethernet address is already known */
        header.dst = _table[next_hop_ip];
        header.src = _ethernet_address;
        header.type = EthernetHeader::TYPE_IPv4;
        frame.payload() = dgram.serialize();

        /* send it right away */
        _frames_out.push(frame);
    } else {
        /* If the destination Ethernet address is unknown */

        /* Except: already sent an ARP with same IP within 5000 ms */
        if (_uniq_ip.find(next_hop_ip) != _uniq_ip.end()) {
            if (_timestamp <= _uniq_ip[next_hop_ip])
                return;
            _uniq_ip.erase(next_hop_ip);
        }

        /* 1. broadcast an ARP request for the next hop's Ethernet address */
        header.dst = _broadcast_address;
        header.src = _ethernet_address;
        header.type = EthernetHeader::TYPE_ARP;

        /* create ARP request massage */
        ARPMessage msg;
        msg.opcode = ARPMessage::OPCODE_REQUEST;
        msg.sender_ethernet_address = _ethernet_address;
        msg.sender_ip_address = _ip_address.ipv4_numeric();
        msg.target_ethernet_address = _unknown_address;
        msg.target_ip_address = next_hop_ip;

        frame.payload() = msg.serialize();
        _frames_out.push(frame);

        /* 2. queue the IP datagram and set expire time */
        _dgram_buffer.push(DGramEntry{_timestamp + 5 * 1000, next_hop, dgram});
        /* 3. add to the unique IP container */
        _uniq_ip[next_hop_ip] = _timestamp + 5 * 1000;
    }

    return;
}

//! \param[in] frame the incoming Ethernet frame
std::optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    const EthernetHeader &header = frame.header();
    bool is_broadcast = header.dst == _broadcast_address;
    bool is_destined = header.dst == _ethernet_address;

    /* ignore any frames not destined for the network interface */
    if (!(is_broadcast || is_destined))
        return std::nullopt;

    if (header.type == EthernetHeader::TYPE_IPv4) {
        InternetDatagram dgram;
        if (dgram.parse(frame.payload()) != ParseResult::NoError)
            return std::nullopt;
        return dgram;
    }

    if (header.type == EthernetHeader::TYPE_ARP) {
        ARPMessage msg;
        if (msg.parse(frame.payload()) != ParseResult::NoError)
            return std::nullopt;

        /* remember the mapping between the sender's IP address and Ethernet address for
30 seconds */
        uint32_t sender_ip_address = msg.sender_ip_address;
        EthernetAddress sender_ethernet_address = msg.sender_ethernet_address;

        _table[sender_ip_address] = sender_ethernet_address;
        _ip_buffer.push(IPEntry{_timestamp + 30 * 1000, sender_ip_address});

        /* delete sender_ip_address in unique container */
        _uniq_ip.erase(sender_ip_address);

        /* if ARP message is a request AND final dest is ME, send an ARP Reply */
        /* ARP Request may be just transit pass ME, I don't send ARP Reply */
        if (msg.opcode == ARPMessage::OPCODE_REQUEST && msg.target_ip_address == _ip_address.ipv4_numeric()) {
            EthernetFrame reply_frame;
            EthernetHeader &reply_header = reply_frame.header();
            reply_header.dst = sender_ethernet_address;
            reply_header.src = _ethernet_address;
            reply_header.type = EthernetHeader::TYPE_ARP;

            /* create ARP reply massage */
            ARPMessage reply_msg;
            reply_msg.opcode = ARPMessage::OPCODE_REPLY;
            reply_msg.sender_ethernet_address = _ethernet_address;
            reply_msg.sender_ip_address = _ip_address.ipv4_numeric();
            reply_msg.target_ethernet_address = sender_ethernet_address;
            reply_msg.target_ip_address = sender_ip_address;

            reply_frame.payload() = reply_msg.serialize();
            _frames_out.push(reply_frame);
        }

        /* send the datagrams in the buffer as much as possible */
        while (!_dgram_buffer.empty()) {
            DGramEntry &e = _dgram_buffer.front();
            uint32_t next_hop_ip = e.next_hop.ipv4_numeric();
            if (_table.find(next_hop_ip) == _table.end())
                break;
            send_datagram(e.dgram, e.next_hop);
            _uniq_ip.erase(next_hop_ip);
            _dgram_buffer.pop();
        }
    }

    return std::nullopt;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    _timestamp += ms_since_last_tick;

    /* remove expired datagrams */
    while (!_dgram_buffer.empty() && _timestamp > _dgram_buffer.front().ttl) {
        DGramEntry &e = _dgram_buffer.front();

        /* retransmit the datagram */
        send_datagram(e.dgram, e.next_hop);

        // we cannot delete in uniq container: _uniq_ip.erase(e.next_hop.ipv4_numeric());
        _dgram_buffer.pop();
    }

    /* remove expired IP-to-Ethernet address mappings */
    while (!_ip_buffer.empty() && _timestamp > _ip_buffer.front().ttl) {
        _table.erase(_ip_buffer.front().ip_address);
        _ip_buffer.pop();
    }

    return;
}
