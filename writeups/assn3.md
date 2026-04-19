Assignment 3 Writeup
=============

My name: Dokyung Kim

My POVIS ID: dokyung03

My student ID (numeric): 20220801

This assignment took me about [8] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPSender:
TCP Sender works by sending the byte stream which Sender received from application. The important things that we must consider are how many bytes we send at once and how we retransmit the segment. First, TCP Sender send SYN segment to hand shake with TCP receiver. The condition of SYN segment is the absolute sequence number is 0. After sending SYN segment, TCP Sender will receive acknowledge segment from TCP receiver with window size and acknowledge sequence number 1. Then TCP sender can send data. TCP Sender push data to queue of sending as much as possible. The limit of quantity of sending data is window size of receiver. Then TCP sender turn on the timer and wait the acknowledge segment. When TCP Sender receive acknowledge segment in time, it checks the acknowledge sequence number. If ackno is same with the last number +1 of TCP Sender sent, it sents the next bytes in byte stream. If not, it sends the oldest segment in the sending queue. If TCP Segment didn't receive acknowledge segment, it turns on the timer again with more time and send again. The waiting time grows twice whenever time out.
I didn't make additional function. All process of code was based on PDF.

Implementation Challenges:
 There are two main functions in tcp_sender.cc. The first one is fill_window(). Its role is to send the data in byte stream. But, it is hard to handle the edge cases. The SYN segment and FIN segment must be considered. Moreover, the FIN signal could be in the payload segment. And I must have made the flag of sender got the eof signal. If the sender got eof signal once, then the sender should never make FIN segment anymore. And, if the window size is zero, we regard it as window size is one. So, it was hard to consider the edge cases in implementing fill_window() function.
 There is another main function which is ack_received. Its role is to receive acknowledge segment and control the sending system. The first challenge in implemening ack_received was checking the queue TCP sender sent. TCP sender must check the ackno and traversal the queue with index. To handling the index was complicated because TCP sender consider the window size and the left edge of window. While TCP sender traversal the queue, if the seqno of member of queue was less than the received ackno, TCP sender should remove it from the queue. After traversal, if there is no member in queue, all things go well, so TCP sender just send next data in byte stream. But, if there are still members in queue, TCP sender must retransmit the segment again. The retransmitting functions was implemented in tick().

Remaining Bugs:
The TCP Sender program (tcp_sender.hh and tcp_sender.cc) is controled by external program. So, just only with TCP Sender program which I implemented, it cannot work correctly. What I mean is When to call tick() and other functions. The parameter of ms_since_last_call is controlled by external program, so I cannot manually control the timer. I should believe that the external program will call tick() at right time. It means if the tick() called sparsely, for example there is no calling after turned on and turned off, the timer variable will be added with too big timer value. It can spoil the sender program.

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
