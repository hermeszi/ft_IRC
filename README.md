*This project has been created as part of the 42 curriculum by <banana1>, <banana2>.*

# ft_irc

## Description

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

Add the important design decisions here.

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

* <banana1>
* <banana2>

---

## License

This project is part of a learning path and is not intended for use.
