<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2 Final//EN">
<!-- Manual page created with latex2man on Thu Aug  2 11:01:24 CEST 2018
** Author of latex2man: Juergen.Vollmer@informatik-vollmer.de
** NOTE: This file is generated, DO NOT EDIT. -->
<html>
<head><title>NOWEBSOCKET</title>
</head><body bgcolor="white">
<h1 align=center>
transparent websocket API 
</h1>
<h4 align=center>C-streamer </h4>
<h4 align=center>02 August 2018</h4>
<h4 align=center>Version </h4>
<p>
This manual page will never be part of the POSIX Programmer's Manual. 
<p>
nowsread, nowswrite, nowsclose - read, write, close a regular stream socket when peer is a websocket. 
<p>
nowsread, nowswrite and nowsclose are designed to be similar to posix read, 
write and close. Their purpose is to provide native sockets to HTML5 web 
browsers. This is not yet allowed by standards, but the other way around is 
not forbidden. These functions mimic posix's read, write and close and 
transparently emulate the websocket protocol. 
<p>
These functions allow their users to communicate with HTML5-enabled web 
browsers using legacy knowledge and habits gained through years of 
experience on network programming with TCP stream sockets and POSIX API as a 
reference. 
<p>
<h2><a name="section_1">SYNOPSYS</a></h2>

<p>
#include <nows.h> 
<p>
ssize_t nowsread (int <i>sock</i>,
void* <i>data</i>,
size_t <i>len</i>);
<p>
ssize_t nowswrite (int <i>sock</i>,
const void* <i>data</i>,
size_t <i>len</i>);
<p>
int nowsclose (int <i>sock</i>);
<p>
int nows_simulate_client (int <i>sock</i>);
<p>
<h2><a name="section_2">DESCRIPTION</a></h2>

<p>
nowsread(3) is very similar to read(3POSIX) in its arguments, behavior, 
return value, in blocking or non blocking mode, when <i>sock</i>
is a STREAMS 
socket in <em>byte-stream</em>
mode (<em>message-nondiscard</em>
and 
<em>message-discard</em>
modes are yet unconsidered). Same goes to 
nowswrite(3) regarding write(3POSIX). The websocket protocol includes a 
handshake process which sadly breaks symetry between peers. As generally 
HTML5 browsers act as websocket clients, nowsread(3) and nowswrite(3) act by 
default as websocket server (even if the socket <i>sock</i>
is a SOCK_STREAM 
client). A call to nows_simulate_client(3) prior to nowsread(3) and 
nowswrite(3) will switch this behavior. 
<p>
Nothing beeing perfect, here is a list of specific differences or relevant notices: 
<p>
<dl compact>
<p>
<dt><em>blocking mode</em>
</dt>
<dd> 
read(3POSIX) blocks the caller until exactly <i>len</i>
bytes are 
received. Similarly nowsread(3) blocks the caller until at least one 
byte is received, but due to the nature of the websocket protocol, this 
does not guarantee that exactly <i>len</i>
bytes are received. The 
returned value indicates the amount of received bytes in the <i>data</i>
buffer. nowswrite(3) has however the same behavior as write(3POSIX). 
<p>
</dd>
<dt><em>non-blocking mode</em>
</dt>
<dd> 
Any value (greater than 0) given to <i>len</i>
will follow 
read/write(3POSIX) behavior. 
<p>
</dd>
<dt><em>websocket specifics or restrictions</em>
</dt>
<dd> 
Underlying ping-pong facility is not implemented yet. Only text buffers are sent 
or received. This is subject for improvements. 
<p>
</dd>
</dl>
<p>
nowsclose(3) is similar to close(3). 
<p>
nows_simulate_client(3) sets this side of the stream to act as a websocket 
client. 
<p>
<h2><a name="section_3">RETURN VALUE</a></h2>

<p>
Please refer to read(3POSIX), write(3POSIX). To summarize, 0 always 
indicates EOF, -1 an error and in that case <i>errno</i>
is set. <i>errno</i>
can be set to EAGAIN even when in blocking mode. However in blocking mode 
EAGAIN is never continuously returned and cannot induce an active loop. 
Otherwise the returned value indicates the number of bytes sent or received. 
<p>
nowsclose(3) is similar to close(3POSIX). 
<p>
now_simulate_client(3) returns 0 on success, -1 on error and in that case 
<i>errno</i>
is set. 
<p>
<h2><a name="section_4">ERRORS</a></h2>

<p>
They are the same as with read(3POSIX), write(3POSIX) and close(3POSIX). 
Additionnaly, 
<p>
<dl compact>
<p>
<dt>ENOMEM</dt>
<dd> 
can be returned in case of memory shortage. 
<p>
</dd>
<dt>EAGAIN</dt>
<dd> 
can be returned by nowsread(3) or nowswrite(3) in blocking mode (however 
never continuously thus not inducing an active loop). This is subject 
for improvements. 
<p>
</dd>
</dl>
<p>
</body>
</html>
<!-- NOTE: This file is generated, DO NOT EDIT. -->
