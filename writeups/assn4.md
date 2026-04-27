Assignment 4 Writeup
=============

My name: Kim Dokyung

My POVIS ID: dokyung03

My student ID (numeric): 20220801

This assignment took me about [12] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Your benchmark results (without reordering, with reordering): [1.20, 0.95]

Program Structure and Design of the TCPConnection:
 There two main logics in this assignment.
 The first one is segment receiving. It should cover many cases. First, it should set error and finish the connection when it gets reset flagged segment. Second, it should receive only SYN segment before socket starts connection. Then it gives segment to receiver and notice the ackno and window size to sender. Third, if there is no element in sending queue so that it cannot send the acknowledgement segment or it receives keep-alive segment, send empty segment. Fourth, if remote peer finished connection first, and local peer finished connection, so it sent fin segment, and all sending segments are acknowledged by remote peer, it should end the TCPConnection. Fifth, if remote peer finished connection first, it should turn off the lingering flag.
 The second main logic is the ending system of connection. There is two cases of end. The first one is unclean shutdown. It is caused by unexpected events. If socket received rst segment from remote peer, it should end the connection. If socket retransmitted same segment more than 8 times, it should end the connection and send RST segment. If the socket destructed by unexpected events, it should end the connection and send RST segment. The second one is clear shutdown. In this case, the connection can be finished in two way. First, if local peer finished first, then it would send FIN segment first. Then the remote peer will send its stream segment and FIN segment. Then local peer will send the acknowledgement segment for remote peer's FIN segment. Then the local peer should wait 10 times of retransmission waiting time. If it doesn't receive any segment, it can end the connection. Second, if remote peer finished first, the local peer send its segments and FIN segment. Then the local peer will receive ACK segment for its FIN segment. After receive it, local peer can end the connection. Both clean shutdown ways are implemented in tick() and segment_received().

Implementation Challenges:
 The most difficult implementation of this assignment was the conditions in segment_received function. If socket receives a segment, it should update its information of ackno and window size of remote peer. So, if the segment is ACK seg, socket should give ackno and window size. Then it should send ACK segment for receiving segment. However, if the receiving segment is the first connection segment, it wouldn't be a ACK seg, but local peer must send ACK about it. So, it should send ACK segment regardless of if receiving segment is ACK or not. However, in this situation, if socket receives non-SYN segment before starting connection, it would send ACK segment. So, I should have implemented exception that before connection, it should neglect all non-SYN segment. For sending ACK segment, it can attach ACK flag to original sending segment. However, if there is no segment in sending queue, due to the payload isn't prepared yet, the local peer cannot attach ACK flag. So, it should make empty packet and attach ACK and send. It is the condition of empty sending queue. And it is possible that socket can receive of keep-alive which has only header and is not SYN or FIN segment. The socket should send empty ACK segment for it.
 The condition that clean shutdown was difficult too. If remote peer finished connection first, the receiver would reassemble packets and set eof flag true. And in this situation, local peer didn't finish the connection, so the sender's eof flag would false. So, we can use this condition to make not use lingering. After local peer finish sending its all packet, the sender's eof flag would be also true. But, it is possible that FIN segment hasn't been sent yet. So, we should make the condition that the number of sender's written bytes is less than 2 of sender's next number of sequence number of last sent segment. It's because sender had sent written bytes and SYN segment and FIN segment. And there is no bytes in flight because in this condition, all sent segment must be acknowledged by remote peer. So, I must have implemented all complicated conditions above.

Remaining Bugs:
 I think there is no bugs because I completed all test cases but there would be some inefficient codes. For example, in the process of noticing ackno and window size to sender, there is already sending segment process in its function, but the socket push segment to queue again because of the exception of receiving before connection. So it is possible that socket calls fill_window() twice. I could fix it but it makes more conditions and branchs and then it can make additional bugs, so I didn't do it.

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
