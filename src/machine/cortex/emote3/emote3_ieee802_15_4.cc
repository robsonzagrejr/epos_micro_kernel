// EPOS EPOSMoteIII (ARM Cortex-M3) 802.15.4 NIC Mediator Implementation

#include <system/config.h>

#ifdef __ieee802_15_4__

#include <machine/cortex/cortex_ieee802_15_4.h>

__BEGIN_SYS

// Class attributes
volatile CC2538RF::Reg32 CC2538RF::Timer::_overflow_count;
volatile CC2538RF::Reg32 CC2538RF::Timer::_ints;
volatile CC2538RF::Timer::Time_Stamp CC2538RF::Timer::_int_request_time;
volatile CC2538RF::Timer::Offset CC2538RF::Timer::_offset;
volatile CC2538RF::Timer::Offset CC2538RF::Timer::_periodic_update;
volatile CC2538RF::Timer::Offset CC2538RF::Timer::_periodic_update_update;
volatile CC2538RF::Timer::Offset CC2538RF::Timer::_periodic_update_update_update;
volatile IC::Interrupt_Handler CC2538RF::Timer::_handler;
bool CC2538RF::Timer::_overflow_match;
bool CC2538RF::Timer::_msb_match;

// TODO: Static because of TSTP_MAC
bool CC2538RF::_cca_done;

IEEE802_15_4_NIC::Device IEEE802_15_4_NIC::_devices[UNITS];

// Methods
IEEE802_15_4_NIC::~IEEE802_15_4_NIC()
{
    db<IEEE802_15_4_NIC>(TRC) << "~IEEE802_15_4_NIC(unit=" << _unit << ")" << endl;
}

int IEEE802_15_4_NIC::send(const Address & dst, const IEEE802_15_4::Type & type, const void * data, unsigned int size)
{
    db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::send(s=" << address() << ",d=" << dst << ",p=" << hex << type << dec << ",d=" << data << ",s=" << size << ")" << endl;

    Buffer * b = alloc(dst, type, 0, 0, size);
    memcpy(b->frame()->data<void>(), data, size);
    return send(b);
}

int IEEE802_15_4_NIC::receive(Address * src, IEEE802_15_4::Type * type, void * data, unsigned int size)
{
    db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::receive(s=" << *src << ",p=" << hex << *type << dec << ",d=" << data << ",s=" << size << ") => " << endl;

    Buffer * buf;
    for(buf = 0; !buf; ++_rx_cur_consume %= RX_BUFS) { // _xx_cur_xxx are simple accelerators to avoid scanning the ring buffer from the beginning.
                                                       // Losing a write in a race condition is assumed to be harmless. The FINC + CAS alternative seems too expensive.
        unsigned int idx = _rx_cur_consume;
        if(_rx_bufs[idx]->lock()) {
            if(_rx_bufs[idx]->size() > 0)
                buf = _rx_bufs[idx];
            else
                _rx_bufs[idx]->unlock();
        }
    }

    Address dst;
    unsigned int ret = MAC::unmarshal(buf, src, &dst, type, data, size);
    free(buf);

    db<IEEE802_15_4_NIC>(INF) << "IEEE802_15_4_NIC::received " << ret << " bytes" << endl;

    return ret;
}

IEEE802_15_4::Buffer * IEEE802_15_4_NIC::alloc(const Address & dst, const IEEE802_15_4::Type & type, unsigned int once, unsigned int always, unsigned int payload)
{
    db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::alloc(s=" << address() << ",d=" << dst << ",p=" << hex << type << dec << ",on=" << once << ",al=" << always << ",ld=" << payload << ")" << endl;

    // Initialize the buffer
    Buffer * buf = new (SYSTEM) Buffer(this, 0);
    buf->size(once + always + payload + sizeof(IEEE802_15_4::Header));
    MAC::marshal(buf, address(), dst, type);

    return buf;
}

int IEEE802_15_4_NIC::send(Buffer * buf)
{
    db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::send(buf=" << buf << ")" << endl;
    db<IEEE802_15_4_NIC>(INF) << "IEEE802_15_4_NIC::send:frame=" << buf->frame() << " => " << *(buf->frame()) << endl;

    unsigned int size = MAC::send(buf);

    if(size) {
        _statistics.tx_packets++;
        _statistics.tx_bytes += size;
    } else
        db<IEEE802_15_4_NIC>(WRN) << "IEEE802_15_4_NIC::send(buf=" << buf << ")" << " => failed!" << endl;

    return size;
}

void IEEE802_15_4_NIC::free(Buffer * buf)
{
    db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::free(buf=" << buf << ")" << endl;

    _statistics.rx_packets++;
    _statistics.rx_bytes += buf->size();

    buf->size(0);
    buf->unlock();
}

void IEEE802_15_4_NIC::reset()
{
    db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::reset()" << endl;

    // Reset statistics
    new (&_statistics) Statistics;
}

bool IEEE802_15_4_NIC::reconfigure(const Configuration & c)
{
    db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::reconfigure(c=" << c << ")" << endl;

    IEEE802_15_4_Engine::address(c.address);
    if(IEEE802_15_4_Engine::address() == c.address)
        _address = c.address;
    else
        return false;

    if((c.channel > 10) && (c.channel < 27)) {
        IEEE802_15_4_Engine::channel(_channel);
        _channel = c.channel;
    } else
        return false;

    // TODO: implement power and period reconfigurations
    return true;
}

void IEEE802_15_4_NIC::configuration(Configuration * c)
{
    db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::configuration(c=" << c << ")" << endl;

    c->channel = _channel;
}

void IEEE802_15_4_NIC::handle_int()
{
    Timer::Time_Stamp sfd = Timer::sfd();

    Reg32 irqrf0 = sfr(RFIRQF0);
    Reg32 irqrf1 = sfr(RFIRQF1);
    Reg32 errf = sfr(RFERRF);
    sfr(RFIRQF0) = irqrf0 & INT_RXPKTDONE; //INT_RXPKTDONE is polled by rx_done()
    sfr(RFIRQF1) = irqrf1 & INT_TXDONE; //INT_TXDONE is polled by tx_done()
    sfr(RFERRF) = errf & (INT_TXUNDERF | INT_TXOVERF);
    if(Traits<IEEE802_15_4_NIC>::hysterically_debugged) {
        db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::handle_int()" << endl;

        db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::handle_int:RFIRQF0=" << hex << irqrf0 << endl;
        db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::handle_int:RFIRQF1=" << hex << irqrf1 << endl;
        db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::handle_int:RFERRF=" << hex << errf << endl;
    }

    if(errf & (INT_RXUNDERF | INT_RXOVERF)) { // RX Error
        CC2538RF::drop();
        IC::enable(IC::INT_NIC0_TIMER);
        db<IEEE802_15_4_NIC>(INF) << "IEEE802_15_4_NIC::handle_int:RFERRF=" << hex << errf << endl;
    } else if(irqrf0 & INT_FIFOP) { // Frame received
        if(Traits<IEEE802_15_4_NIC>::hysterically_debugged)
            db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::handle_int:receive()" << endl;
        if(CC2538RF::filter()) {
            Buffer * buf = 0;
            unsigned int idx = _rx_cur_produce;
            for(unsigned int count = RX_BUFS; count; count--, ++idx %= RX_BUFS) {
                if(_rx_bufs[idx]->lock()) {
                    buf = _rx_bufs[idx];
                    break;
                }
            }
            _rx_cur_produce = (idx + 1) % RX_BUFS;

            if(buf) {
                buf->size(CC2538RF::copy_from_nic(buf->frame()));
                // When AUTO_CRC is on, the radio automatically puts the RSSI on the second-to-last byte
                assert(xreg(FRMCTRL0) & AUTO_CRC);
                assert(buf->size() >= 2);
                buf->rssi = reinterpret_cast<char *>(buf->frame())[buf->size() - 2];
                buf->sfd_time_stamp = sfd;

                if(MAC::pre_notify(buf)) {
                    db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::handle_int:receive(b=" << buf << ") => " << *buf << endl;
                    IC::enable(IC::INT_NIC0_TIMER); // Make sure radio and MAC timer don't preempt one another
                    bool notified = notify(reinterpret_cast<IEEE802_15_4::Header *>(buf->frame())->type(), buf);
                    if(!MAC::post_notify(buf) && !notified)
                        buf->unlock(); // No one was waiting for this frame, so make it available for receive()
                } else {
                    db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::handle_int: frame dropped by MAC"  << endl;
                    buf->size(0);
                    buf->unlock();
                    IC::enable(IC::INT_NIC0_TIMER); // Make sure radio and MAC timer don't preempt one another
                }
            } else {
                db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::handle_int: dropped" << endl;
                CC2538RF::drop();
                IC::enable(IC::INT_NIC0_TIMER); // Make sure radio and MAC timer don't preempt one another
            }
        } else {
            db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::handle_int: not filtered" << endl;
            IC::enable(IC::INT_NIC0_TIMER); // Make sure radio and MAC timer don't preempt one another
        }
    } else {
        db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::handle_int: NOT FIFOP" << endl;
        IC::enable(IC::INT_NIC0_TIMER);
    }
}

void IEEE802_15_4_NIC::int_handler(IC::Interrupt_Id interrupt)
{
    IEEE802_15_4_NIC * dev = get_by_interrupt(interrupt);

    db<IEEE802_15_4_NIC>(TRC) << "Radio::int_handler(int=" << interrupt << ",dev=" << dev << ")" << endl;

    if(!dev)
        db<IEEE802_15_4_NIC>(WRN) << "Radio::int_handler: handler not assigned!" << endl;
    else
        dev->handle_int();
}

#ifdef __tstp__

// TSTP binding
//template<typename Radio>
//void TSTP::MAC<Radio, true>::free(Buffer * b) { IEEE802_15_4_NIC::get()->free(b); }
//
//template<typename Radio>
//void TSTP::MAC<Radio, false>::free(Buffer * b) { IEEE802_15_4_NIC::get()->free(b); }
//
//template<typename Radio>
//bool TSTP::MAC<Radio, true>::equals(Buffer * b0, Buffer * b1)
//{
//    if(b0->id != b1->id)
//        return false;
//    Header * header = b0->frame()->data<Header>();
//    Header * other_header = b1->frame()->data<Header>();
//    return
//        (other_header->version() == header->version()) &&
//        (other_header->type() == header->type()) &&
//        (other_header->scale() == header->scale()) &&
//        (other_header->time() == header->time()) &&
//        (other_header->origin() == header->origin());
//}
//
//template<typename Radio>
//bool TSTP::MAC<Radio, false>::equals(Buffer * b0, Buffer * b1)
//{
//    if(b0->id != b1->id)
//        return false;
//    Header * header = b0->frame()->data<Header>();
//    Header * other_header = b1->frame()->data<Header>();
//    return
//        (other_header->version() == header->version()) &&
//        (other_header->type() == header->type()) &&
//        (other_header->scale() == header->scale()) &&
//        (other_header->time() == header->time()) &&
//        (other_header->origin() == header->origin());
//}

#endif

__END_SYS

#endif
