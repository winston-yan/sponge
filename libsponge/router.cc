#include "router.hh"

#include <iostream>

// Given an incoming Internet datagram, the router decides
// (1) which interface to send it out on, and
// (2) what next hop address to send it to.

//! \param[in] route_prefix The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
//! \param[in] prefix_length For this route to be applicable, how many high-order (most-significant) bits of the route_prefix will need to match the corresponding bits of the datagram's destination address?
//! \param[in] next_hop The IP address of the next hop. Will be empty if the network is directly attached to the router (in which case, the next hop address should be the datagram's final destination).
//! \param[in] interface_num The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const std::optional<Address> next_hop,
                       const size_t interface_num) {
    std::cerr << "DEBUG: adding route " << Address::from_ipv4_numeric(route_prefix).ip() << "/" << int(prefix_length)
              << " => " << (next_hop.has_value() ? next_hop->ip() : "(direct)") << " on interface " << interface_num
              << std::endl;

    _table.push_back(RouteEntry{route_prefix, prefix_length, next_hop, interface_num});
    return;
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    /* decrement TTL, DROP dgram if TTL equal to zero */
    if (dgram.header().ttl == 0 || --dgram.header().ttl == 0)
        return;

    size_t max_len = 0;
    uint32_t dst_ip_address = dgram.header().dst;
    bool found_match = false;
    int match_index = 0, n = _table.size();
    for (int i = 0; i < n; ++i) {
        RouteEntry &e = _table[i];
        if (e.prefix_length < max_len)
            continue;
        if (e.prefix_length == 0 || ((dst_ip_address ^ e.route_prefix) >> (32 - e.prefix_length)) == 0) {
            found_match = 1;
            max_len = e.prefix_length;
            match_index = i;
        }
    }

    /* DROP dgram: no entry match the destination ip address */
    if (!found_match)
        return;

    const RouteEntry &e = _table[match_index];
    interface(e.interface_num)
        .send_datagram(dgram,
                       e.next_hop.has_value() ? e.next_hop.value() : Address::from_ipv4_numeric(dgram.header().dst));

    return;
}

void Router::route() {
    // Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());
            queue.pop();
        }
    }
}
