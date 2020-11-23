// EPOS EPOSMoteIII (ARM Cortex-M3) 802.15.4 NIC Mediator Initialization

#include <system.h>

#ifdef __ieee802_15_4__

#include <machine/gpio.h>
#include <machine/cortex/cortex_ieee802_15_4.h>

__BEGIN_SYS

IEEE802_15_4_NIC::IEEE802_15_4_NIC(unsigned int unit): _unit(unit), _rx_cur_consume(0), _rx_cur_produce(0)
{
    db<IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC(unit=" << unit << ") => " << this << endl;

    // Initialize RX buffer pool
    for(unsigned int i = 0; i < RX_BUFS; i++)
        _rx_bufs[i] = new (SYSTEM) Buffer(0, 0);

    // Put the radio in FULL power mode (at this point, it was OFF)
    power(FULL);

    // Set Address
    const UUID & id = Machine::uuid();
    _address[0] = id[4] ^ id[5];
    _address[1] = id[6] ^ id[7];
    ffsm(SHORT_ADDR0) = _address[0];
    ffsm(SHORT_ADDR1) = _address[1];

    xreg(FRMFILT1) &= ~ACCEPT_FT2_ACK; // ACK frames are handled only when expected
    xreg(FRMCTRL0) |= AUTO_CRC; // Enable auto-CRC

    if(!Traits<IEEE802_15_4_NIC>::tstp_mac) {
        // Configure frame filtering by hardware
        if(CC2538RF::promiscuous)
            xreg(FRMFILT0) &= ~FRAME_FILTER_EN;
        else
            xreg(FRMFILT0) |= FRAME_FILTER_EN;
        xreg(SRCMATCH) |= SRC_MATCH_EN; // Enable automatic source address matching
        xreg(FRMCTRL0) |= AUTO_ACK; // Enable auto ACK
        xreg(RXMASKSET) = RXENABLE_STXON; // Enter receive mode after ISTXON
    } else {
        xreg(FRMFILT0) &= ~FRAME_FILTER_EN; // Disable frame filtering
        xreg(SRCMATCH) &= ~SRC_MATCH_EN; // Disable automatic source address matching
        if(Traits<System>::DUTY_CYCLE == 1000000)
            xreg(RXMASKSET) = RXENABLE_STXON; // Enter receive mode after ISTXON
        else
            xreg(FRMCTRL1) &= ~SET_RXENMASK_ON_TX; // Do not enter receive mode after ISTXON
    }

    channel(Traits<IEEE802_15_4_NIC>::DEFAULT_CHANNEL);

    if(Traits<IEEE802_15_4_NIC>::gpio_debug) {
        // Enable debug signals to GPIO
        GPIO p_tx('C',3,GPIO::OUT,GPIO::FLOATING); // Configure GPIO pin C3
        GPIO p_rx('C',5,GPIO::OUT,GPIO::FLOATING); // Configure GPIO pin C5
        if(Traits<IEEE802_15_4_NIC>::promiscuous) {
            xreg(RFC_OBS_CTRL0) = SIGNAL_RX_ACTIVE; // Signal 0 is RX_ACTIVE
            xreg(RFC_OBS_CTRL1) = SIGNAL_TX_ACTIVE; // Signal 1 is TX_ACTIVE
        } else {
            xreg(RFC_OBS_CTRL0) = SIGNAL_TX_ACTIVE; // Signal 0 is TX_ACTIVE
            xreg(RFC_OBS_CTRL1) = SIGNAL_RX_ACTIVE; // Signal 1 is RX_ACTIVE
        }
        cctest(CCTEST_OBSSEL3) = OBSSEL_SIG0_EN; // Route signal 0 to GPIO pin C3
        cctest(CCTEST_OBSSEL5) = OBSSEL_SIG1_EN; // Route signal 1 to GPIO pin C5
    }

    reset(); // Reset statistics

    // Clear interrupts
    sfr(RFIRQF0) = 0;
    sfr(RFIRQF1) = 0;

    // Clear error flags
    sfr(RFERRF) = 0;

    // Enable useful device interrupts
    // INT_TXDONE, INT_TXUNDERF, and INT_TXOVERF are polled by CC2538RF::tx_done()
    // INT_RXPKTDONE is polled by CC2538RF::rx_done()
    xreg(RFIRQM0) = INT_FIFOP;
    xreg(RFIRQM1) = 0;
    xreg(RFERRM) = (INT_RXUNDERF | INT_RXOVERF);


    MAC::constructor_epilogue(); // Device is configured, let the MAC use it
}


void IEEE802_15_4_NIC::init(unsigned int unit)
{
    db<Init, IEEE802_15_4_NIC>(TRC) << "IEEE802_15_4_NIC::init(unit=" << unit << ")" << endl;

    // Initialize the device
    IEEE802_15_4_NIC * dev = new (SYSTEM) IEEE802_15_4_NIC(unit);

    Timer::init();

    // Register the device
    _devices[unit].device = dev;
    _devices[unit].interrupt = IC::INT_NIC0_RX;
    _devices[unit].error_interrupt = IC::INT_NIC0_ERR;

    // Install interrupt handler
    IC::int_vector(_devices[unit].interrupt, &int_handler);
    IC::int_vector(_devices[unit].error_interrupt, &int_handler);

    // Enable interrupts for device
    IC::enable(IC::INT_NIC0_RX);
    IC::enable(IC::INT_NIC0_ERR);
}


void IEEE802_15_4_NIC::Timer::init()
{
    db<Init, IEEE802_15_4_NIC>(TRC) << "Radio::Timer::init()" << endl;
    _periodic_update = 0;
    _periodic_update_update = 0;
    _periodic_update_update_update = 0;
    mactimer(MTCTRL) &= ~MTCTRL_RUN; // Stop counting
    mactimer(MTIRQM) = 0; // Mask interrupts
    mactimer(MTIRQF) = 0; // Clear interrupts
    mactimer(MTCTRL) &= ~MTCTRL_SYNC; // We can't use the sync feature because we want to change the count and overflow values when the timer is stopped
    mactimer(MTCTRL) |= MTCTRL_LATCH_MODE; // count and overflow will be latched at once
    IC::int_vector(IC::INT_NIC0_TIMER, &int_handler); // Register and enable interrupt at IC
    IC::enable(IC::INT_NIC0_TIMER);
    int_enable(INT_OVERFLOW_PER); // Enable overflow interrupt
    mactimer(MTCTRL) |= MTCTRL_RUN; // Start counting
}

__END_SYS

#endif
