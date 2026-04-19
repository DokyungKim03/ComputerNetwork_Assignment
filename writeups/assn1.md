Assignment 1 Writeup
=============

My name: Kim Dokyung

My POVIS ID: dokyung03

My student ID (numeric): 20220801

This assignment took me about [6] hours to do (including the time on studying, designing, and writing the code).

Program Structure and Design of the StreamReassembler:
The process of Reassembling has 5 steps.
1. The sum of the size of reassembled substrings and substrings not reassembled(=buffer) should be smaller than or equal to capacity. So, first I made the buffer size as capacity.
2. When it receives the substring, substring must be sliced to fit in the buffer.
3. Then the substring finds appropriate index to be stored.
4. If there is already a string in the index, it can't be stored and moves to next index.
5. After inserting into buffer, it checks the first member of buffer is filled, and if so, it carries continuing string to _output.

Implementation Challenges:
When I implemented it at first, I made the buffer by string. It was hard to find appropriate index and distinguish the substrings. Then I researched C++ data structures, and I could find the well fitted structure which was deque. It could use memory dynamically, and it didn't violate the assignment C++ format.
There is another challenge, how to use eof. I misunderstood about eof as it meant the last input of the files. But, its meaning was the last file after reassembling. However, It was also hard thing. When should I end the input? To solve this problem, I made additionary variables of recording the index of last file. Whenever it moves the substrings to _output, it should check if the moving index is equal to recorded index of last file or not. If so, it moves the file and call the end_input of the bytestream.

Remaining Bugs:
At assignment 0, I use string function, substr. If I want to slice the string, then I use str1 = str.substr(size). It makes another string, so if the size of string is very large, it can have large time complexity.

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
