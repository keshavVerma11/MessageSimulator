Message System Details/How it works:

This project simulates a user and server message protocol for three users, where the users are processing messages received and sending messages simultaneously. To implement this, three child processes were forked from the main process, which we refer to as the server process. Additionally 6 pipes were created. Three for communication from user_to_server and three for communication from server_to_user. Before forking the processes, only the file descriptors (pipe ends) that were allowed for each user were kept oper for each user, and the same can be said about the server. 

Each user (child process) would send messages to the server by calling the user handler which would process the array of messages passed by writing them to the proper user_to_server pipe, and then send a signal corresponding to that user, which would increment a global variable, indicating that a message was sent. For each user, there were 2 global variables (6 total), one telling the server how many messages were sent by the user, and one telling the user how many messages to receive (we will get to the latter later). After sending all the messages, the user would close the write end of the pipe and send another signal to indicate to the server that the user is done sending messages.

Once messages are sent to the server, the server reads each message by accessing the read end of the user_to_server pipe using read(), and then immediately relays the message back to the user by using the write() function by writing to the server_to_user pipe and sending a signal to increment the global variable that indicates number of messages to receive for that user. Once all messages are forwarded, the write ends of the server_to_user pipe are closed and another signal is sent (this time to each user process), to indicate that all the messages have been sent to the recipient. 

Back in the User handler, now each user process will read from the server_to_user pipe similar to the way the server read from user_to_server, and output the message it received displaying the user that sent it the message. Once every message has been received, the server process will reap all of its children's processes and terminate the program!

------
In which cases may read() not read as expected when the number of bytes is set to a fixed number and why does this happen(2.2)? How was it fixed?

1. In the case that it worked properly and you were lucky, it was because the number of bytes that read() read was exactly the number that was written to the pipe.  
2. In the case messages got mixed up but you can still distinguish, it was because both messages themselves add up to more than the number of bytes read but each message itself was less than the number of bytes
3. In the case that messages got concatenated and read() then blocks, it was because both messages when combined are still less than the count number of bytes written so for the second read call which blocks, it is trying to read an empty pipe and there is nothing to read because both messages were accidentally read in the first read call.

To fix this, I wrote 128 bytes every time, padding the message will extra NULL characters to indicate that the rest of the message chars starting at the first NULL are not to be read.

------
Why may you lose messages and how might you fix them(2.3)? 

When a regular signal is sent, during the execution of that signal handler or the sending of the signal, another signal may also be sent and interrupt the previous signal/process and cause that signal to be lost. As a result, the server will not know that there may be more messages to be relayed and the message will simply be discarded. To fix this, instead of using signals like SIGUSR1, SIGUSR2, etc. I used SIGRTMIN, SIGRTMIN+n (n is arbitrary for as many signals as you may need), as these signals can queue one after another so instead of interrupting a signal, it will simply wait for after any previous signals are handled.
