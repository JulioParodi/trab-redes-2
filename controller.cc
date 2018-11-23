#include <iostream>
#include <cmath>
#include <sys/time.h>
#include <stdio.h>


#include "controller.hh"
#include "timestamp.hh"

using namespace std;


long contimeOut = 0;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
{
  initController();
}

/* Get current window size, in datagrams */
unsigned int Controller::window_size()
{
  /* Default: fixed window size of 100 outstanding datagrams */
  unsigned int the_window_size = (unsigned int) cwnd;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << the_window_size << endl;
  }

  return the_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp,
                                    /* in milliseconds */
				    const bool after_timeout
				    /* datagram was sent because of a timeout */ )
{
   /*Default: take no action */
   if (after_timeout) {
     //printf("%s\n", "TIMEOUT ##############");
     DELTA = 0.5;
     cwnd = cwnd * 0.66;
     if (cwnd < 1) cwnd = 1; // Por segurança
     if (contimeOut > 3) cwnd = 1;
     contimeOut++;
   }
   //printf("%ld\n",contimeOut );

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << " (timeout = " << after_timeout << ")\n";
  }
}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  /* Default: take no action */

  contimeOut = 0;
  double lambda , lambda1, dq;
  int rttCurrent ;

  struct timeval timeCurrent;

  rttCurrent = timestamp_ack_received - send_timestamp_acked;

  if (first){
    rttMax = rttCurrent;
    rttMin = rttCurrent;
    rttStanding = rttCurrent;
    srtt = rttCurrent;
    t = srtt / 2;

    first = false;

  }

  srtt = (1 - ALPHA_SRTT) * srtt + ALPHA_SRTT * rttCurrent;

  //t = 300; // em ms
//
// calcular rttStanding (em um tempo t) e rttMin (em um tempo TIME_RTT_MIN)
gettimeofday(&timeCurrent, NULL);
// printf("------------------  %s %ld ---------------------\n","Time ", timeCurrent.tv_sec - timeExec );
// printf("srtt = %d, t = %ld\n",srtt, t );
// printf("rttStanding %d, rttMin %d , rttCurrent %d\n",rttStanding ,rttMin, rttCurrent);
// printf("time rttStanding = %ld\n", (timeCurrent.tv_usec/1000 - oldTime_rttStanding/1000));
  // Calculando rttStanding em um intervalo de tempo T em milissegundos
  if ((timeCurrent.tv_usec/1000 - oldTime_rttStanding/1000) < t){
    if (rttCurrent < rttStanding){
      rttStanding = rttCurrent;
    }
  } else {
    oldTime_rttStanding = timeCurrent.tv_usec;
    rttStanding = rttCurrent;
    t = srtt / 2;
    //printf("%ld  mudeiiiiiiii !!!!!!!!!!! -----------------------------------------------------\n",t );

  }

//gettimeofday(&timeCurrent, NULL);
//printf("time rttMin = %ld\n", timeCurrent.tv_sec - oldTime_rttMin);
  // Calculando rttMIN em um intervalo de tempo TIME_RTT_MIN em segundos
  if ((timeCurrent.tv_sec - oldTime_rttMin) < TIME_RTT_MIN){
    if (rttCurrent < rttMin){
      rttMin = rttCurrent;
    }
  } else {
    oldTime_rttMin = timeCurrent.tv_sec;
    rttMin = rttCurrent;
  }

//-------------------------------------------------------------------------

  dq = rttStanding - rttMin;
// ----- COMPETIVE MODE -----------
  if (contRttMax < 5 ){
    if (rttCurrent > rttMax) rttMax = rttCurrent;
  } else {

    contRttMax = 0;
    rttMax = rttCurrent;
  }
  contRttMax++;



  if (dq < 0.1 * (rttMax - rttMin)){
    DELTA = 0.5;
  }else {
    if (dq < 0.9 * (rttMax - rttMin)){
      DELTA += 0.0005;
    }
      else {
        DELTA *= 0.98;

      }
    }
  if (DELTA > 0.5) DELTA = 0.5;
  if (DELTA < 0.1) DELTA = 0.1;


  if (rttCurrent < 45){
         cwnd += 0.2;
       }else {
         if (rttCurrent < 50){
           cwnd += 0.1;
         }else {
           if (rttCurrent < 55){
             cwnd += 0.05;
           }
         }
       }


  // if (rttCurrent < 30){
  //   //DELTA = 0.1;
  //   cwnd += 0.6;
  // }else {
  //   if (rttCurrent < 35) { // -10 em todos os tempo
  //     // DELTA = 0.25;
  //     cwnd += 0.5;
  //   }else {
  //     if (rttCurrent < 40 ){
  //       cwnd += 0.4;
  //       // DELTA = 0.3;
  //     }else {
  //       if (rttCurrent < 45){
  //         cwnd += 0.3;
  //         // DELTA = 0.3;
  //       }else {
  //         if (rttCurrent < 50){
  //           cwnd += 0.2;
  //           // DELTA = 0.4;
  //         }else {
  //           if (rttCurrent < 55){
  //             cwnd += 0.1;
  //             // DELTA = 0.4;
  //           }else {
  //             // DELTA = 0.5;
  //           }
  //         }
  //       }
  //     }
  //   }
  // }

  printf("%lf\n",DELTA );
  //

  printf("%d\n", rttMin );
//------------------------------------------------------------------------------------------------------

  lambda1 = 1 / ( DELTA * dq );
  lambda = cwnd / rttStanding;

  oldCwnd =  cwnd;
  //if (!aimd) {
  //printf("%s\n", "ESTOU EM COPAA");
  if (lambda <= lambda1){
    cwnd = cwnd + v / (DELTA * cwnd);
  }else {
    cwnd = (cwnd - v / (DELTA * cwnd)) - 0.2 ;
  }
  //}

  if (cwnd < 1) cwnd = 1; // Por segurança

// calcular v (paramentro de velocidade)
  directionOld = direction;
  //printf("oldCwnd = %d, cwnd = %0.f\n", oldCwnd, cwnd );
  if (oldCwnd <  cwnd){
    direction = true;
  } else {
    direction = false;
  }

  if (directionOld == direction){
    if (cont % 3 == 0){ // melhor caso (&& cont < 10)
       v = v * 2;
     }
     if (cont > 8) v = 1; //10 - 24 pow; 13 - 26 pow
     cont++;
     // if (cont > contMax) contMax = cont;
  } else {
    v = 1;
    cont = 1;
  }



  //printf(" v = %d, cont = %d\n",v,cont );
  //printf("%s\n\n","##############################" );
//---------------------------------------

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms()
{
  return 80; /* timeout of one second */
}

void Controller::initController()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  oldTime_rttMin = tv.tv_sec;
  oldTime_rttStanding = tv.tv_usec;
  timeExec = tv.tv_sec;
}
