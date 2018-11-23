#ifndef CONTROLLER_HH
#define CONTROLLER_HH

#include <cstdint>

/* Congestion controller interface */

class Controller
{
private:
  bool debug_; /* Enables debugging output */

  /* Add member variables here */

    const float ALPHA_SRTT = 0.125;
    const unsigned int TIME_RTT_MIN = 10; // segundos
    double DELTA = 0.5; // Default 0.5, mas possui o modo competitivo

    double cwnd = 50.0;
    double oldCwnd = 50.0;
    int rttMin = 0, rttMax = 0;
    int v = 1, cont = 0;
    int contRttMax = 0;
    int rttStanding = 0, srtt = 0;
    unsigned long oldTime_rttMin = 0, oldTime_rttStanding = 0, timeExec = 0;
    unsigned long t = 1;
    bool first = true;
    bool direction = true, directionOld = true;
    //bool aimd = false;

public:

  void initController();

  /* Public interface for the congestion controller */
  /* You can change these if you prefer, but will need to change
     the call site as well (in sender.cc) */


  /* Default constructor */
  Controller( const bool debug );

  /* Get current window size, in datagrams */
  unsigned int window_size();

  /* A datagram was sent */
  void datagram_was_sent( const uint64_t sequence_number,
			  const uint64_t send_timestamp,
			  const bool after_timeout );

  /* An ack was received */
  void ack_received( const uint64_t sequence_number_acked,
		     const uint64_t send_timestamp_acked,
		     const uint64_t recv_timestamp_acked,
		     const uint64_t timestamp_ack_received );

  /* How long to wait (in milliseconds) if there are no acks
     before sending one more datagram */
  unsigned int timeout_ms();
};

#endif
