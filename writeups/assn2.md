Assignment 2 Writeup
=============

My name: Kim Dokyung

My POVIS ID: dokyung03

My student ID (numeric): 20220801

This assignment took me about [8] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
 For wrap and unwrap, I focused on the mapping between sequence numbers and absolute sequence numbers. In wrap function, the function must map absolute 64-bit sequence number to 32-bit sequence number. Because 2^32 is the factor of 2^64, 64-bit numbers can be transformed to 32-bit by cutting off the first 32 bits. So, we can transform sequence number to absolute sequence number just by adding isn and type casting.

 In unwrap function, the function was more complicated than wrap function. It was because we must have mapped the numbers close to checkpoint number. It could make underflow, because subtracting ISN could make minus value and also subtracting 2^32 to sequence number to make the number close to checkpoint could make minus value. The minus values in unsigned type make underflow. That's why I compared the numbers to 2^32.
 
 In tcp_receiver, its design was not that complicated. If I have known well about the sturcture of TCP structure, especially about the header, it was not that hard. I designed flags of SYN and FIN, and used them to make different index of byte. Because SYN and FIN had 1 byte each, I must have added to index 1 for each flag detected. Then, I pushed it to reassembler.


Implementation Challenges:
 When I implemented wrap function, it was simple work because only subtracting and type casting was needed. However implementing unwrap function was harder. It was because the type of final result was unsigned. It means I must have considerd about underflow even in middle procedures. That's why I used ternary operators many times. Also it was hard that considering the case that subtracting 2^32 to transformed number made it closer to checkpoint. If the number was too small to have been minus, it made underflow. The case occur when the subtracted number is closer to checkpoint than original number, so I must have compared both case. Then I can have set the subtraced number to differance between original number and checkpoint + 1 when the number was less than 2^32. It prevented underflow.
 The implement of TCP receiver was simple. The hard things in implementation was just aligning correctness of indecies and types. The wrapping functions was getting various types for inputs, and making various types for outputs. And to find the indecies such as first unassembled or first unacceptable, I must have accessed to past sources like reassembler and byte_stream. Finding the path of variables was challenge in implementation.

Remaining Bugs:
In TCP receiver, If there are duplicated SYN flags, it could make bugs because the receiver sets ISN when SYN flag is received. But, I make syn_received flags in private member to check if SYN flag was received before. So, the bug was solved.

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
