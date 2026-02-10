*This project has been created as part of the 42 curriculum by banana1🍌, banana2🍌.

# 📟 ft_irc

## mingde's comment
**can add 0_NONBLOCK to any exisitng flags**
```
int flags = fcntl(sockfd, F_GETFL, 0);
fcntl(sockfd, F_SETFL, flags | O_NONBLOCK); // Set non-blocking
```
**throw in server init instead of exit (← Doesn't call destructor)**
```
if (_server_fd < 0)
    throw std::runtime_error("socket creation failed");
```

## 📜 Description

**ft_irc** is a 42 School project where you build a minimal IRC (Internet Relay Chat) server in **C++ (C++98)**.

**Goal:** implement a working IRC server that can accept multiple clients, handle core IRC commands, and allow users to chat in channels—without relying on external IRC server code.

**Overview:**

* A TCP server that listens on a port
* Handles multiple clients concurrently (non-blocking I/O + `poll()`/`select()` depending on your implementation)
* Parses IRC messages (`\r\n` terminated)
* Supports a subset of IRC commands (as required by the subject)
* Manages users, nicknames, channels, and permissions (operators, invites, topics, etc.)

> UPDATING

---

## Features

<still populating>

**Networking & I/O**

* [ ] Non-blocking sockets
* [ ] `poll()` based event loop
* [ ] Partial read buffering + message framing (`\r\n`)
* [ ] Graceful disconnect handling

**Core IRC behavior**

* [ ] Password authentication (`PASS`)
* [ ] Nickname and user registration (`NICK`, `USER`)
* [ ] Join/leave channels (`JOIN`, `PART`)
* [ ] Private and channel messages (`PRIVMSG`)
* [ ] Channel modes (e.g. `+i`, `+t`, `+k`, `+o`, `+l`) as required
* [ ] Operator actions (kick/invite/topic)

---

## Project Structure

A simple example layout (tentative):

```
.
├── Makefile
├── inc/
│   ├── Server.hpp
│   ├── Client.hpp
│   ├── Channel.hpp
│   └── ...
├── src/
│   ├── main.cpp
│   ├── Server.cpp
│   ├── Client.cpp
│   ├── Channel.cpp
│   ├── commands/
│   │   ├── Pass.cpp
│   │   ├── Nick.cpp
│   │   └── ...
│   └── ...
└── README.md
```

---

## Instructions

### Requirements

* A C++ compiler supporting **C++98** (e.g. `c++` / `g++` / `clang++`)
* Unix-like environment (Linux / macOS)
* An IRC client for testing (examples below)

### Build

```bash
make
```

### Clean

```bash
make clean
make fclean
make re
```

### Run

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 mypassword
```

---

## Usage

### Connect with an IRC client

**HexChat** (GUI):

* Address: `127.0.0.1`
* Port: `6667`
* Password: `mypassword`

**irssi** (CLI):

```bash
irssi
```

Inside irssi:

```text
/connect 127.0.0.1 6667 mypassword
/nick mynick
/join #test
/msg #test hello
```

**netcat** (raw testing):

```bash
nc 127.0.0.1 6667
PASS mypassword
NICK mynick
USER myuser 0 * :My Real Name
JOIN #test
PRIVMSG #test :hello
```

> Note: IRC lines must end with `\r\n`.

---

## Supported Commands

| Command | Status | Notes                      |
| ------- | -----: | -------------------------- |
| PASS    |      ⬜ | Required for registration? |
| NICK    |      ⬜ | Nick collision handling    |
| USER    |      ⬜ | Registration flow          |
| JOIN    |      ⬜ | Channel creation rules     |
| PART    |      ⬜ |                            |
| PRIVMSG |      ⬜ | Private + channel messages |
| KICK    |      ⬜ |                            |
| INVITE  |      ⬜ |                            |
| TOPIC   |      ⬜ |                            |
| MODE    |      ⬜ | Which modes implemented    |
| QUIT    |      ⬜ | Cleanup / broadcast quit   |

Legend: ⬜ not done, 🟨 partial, ✅ done

---

## Technical Notes

### 👶 Basic Socket event loop

##### - Server init -
1. __socket()__  → Create socket fd
2. setsockopt() // set port to be released after exit (faster testing)
3. fcntl() // Non-blocking
4. __bind()__    → Attach to port
5. __listen()__  → Mark as passive (accepting connections)
6. signal() // Signal handlers
7. //Add to poll array

##### - Server run -
1. poll()  Wait for events on any file descriptor in the list.
2. Check revents & POLLIN → Identify which FD has data ready to read.
3. If FD is _server_fd (New Connection):
'''
    accept() → Create a new connection FD for the client.
    fcntl() → Set the new client FD to O_NONBLOCK.
    New Client Object → Store client data (IP, FD) in _clients map.
    Update pollfds → Add the new FD to the poll array to watch for messages.
'''

4. If FD is a Client FD (Existing Connection):
'''
    recv() → Read incoming bytes into a temporary buffer.
    Check for Disconnect → If recv returns ≤0, run closeClient() and remove from the poll array.
    appendBuffer() → Add raw data to the specific Client object's buffer.
    hasLine() / extractLine() → Loop through the buffer to find complete messages (ending in \n).
    parseMessage() → Process each extracted command (e.g., NICK, JOIN, PRIVMSG).
    Check for Removal → If the command (like QUIT) closed the connection, remove the FD from the poll array immediately.
'''

##### - Client -
1. socket()  → Create socket fd
2. connect() → Connect to server's IP:port
3. send()    → Write data to server
4. recv()    → Read data from server
5. close()   → Clean up

### Message parsing

* How you buffer partial reads
* How you split by `\r\n`
* How you handle multiple messages received in one `recv()`

### Event loop

* `poll()`/`select()` strategy
* How you detect disconnects and errors
* How you avoid blocking writes (etc)

### State model

* Data structures for clients/channels
* Ownership rules (who frees what)
* Where you enforce permissions (ops, invite-only, topic lock, etc.)

---

## Testing


* [ ] Basic connect/register
* [ ] Multiple clients chat in a channel
* [ ] Join/part/quit messages
* [ ] Mode changes behave correctly
* [ ] Stress test: many clients / rapid messages
* [ ] Invalid input: malformed commands, missing params

---

## Resources

### IRC protocol references

* RFC 1459 — Internet Relay Chat Protocol (classic reference) https://tools.ietf.org/html/rfc1459
* RFC 2812 — Internet Relay Chat: Client Protocol https://tools.ietf.org/html/rfc2812
* RFC 2813 — Internet Relay Chat: Server Protocol (extra context)
* Modern IRC Client Protocol: A more readable guide to how modern clients interact with servers https://ircdocs.horse/specs/
* Beej's Guide to Network Programming: A classic reference for socket programming https://beej.us/guide/bgnet/

### Socket programming / multiplexing

* `man 2 socket`, `man 2 bind`, `man 2 listen`, `man 2 accept`, `man 2 recv`, `man 2 send`
* `man 2 poll` (or `man 2 select` if you use select)
* Beej’s Guide to Network Programming

### Useful IRC tooling

* HexChat / irssi usage docs
* netcat basics for line-based TCP testing

### How AI was used

* Used ChatGPT to generate an initial README template and checklist sections.
* Documentation: Generating input for this README.md and verifying RFC command syntax.
* Used AI to brainstorm edge cases for message framing (`\r\n`) and multi-client behavior.
* Used AI to help reword documentation and comment blocks.
* Debugging: Troubleshooting specific edge cases in string parsing (e.g., handling \r\n vs \n).



**AI was NOT used for:**

* Implementing protocol handlers
* Socket management implementation

---

## Authors

* banana1🍌
* banana2🍌

---

## License

This project is part of a learning path and is not intended for use.
