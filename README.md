*This project has been created as part of the 42 curriculum by jngew🍌, myuen🍌.

# 📟 ft_irc

## 📜 Description

**ft_irc** is a 42 School project where you build a minimal IRC (Internet Relay Chat) server in **C++ (C++98)**.

**Goal:** implement a mini working IRC server that can accept multiple clients, handle core IRC commands, and allow users to chat in channels.

---

## Features

**Core IRC Server**
- Multi-client support with non-blocking I/O via `poll()`
- User authentication and registration (PASS → NICK → USER flow)
- Channel creation and management
- Private and channel messaging
- Operator privileges and channel controls

**Implemented Commands**
- Authentication: `PASS`, `NICK`, `USER`
- Channels: `JOIN`, `PART`, `QUIT`
- Messaging: `PRIVMSG`, `PING`/`PONG`

**Channel Operator Commands**
- `KICK` - Remove user from channel
- `INVITE` - Invite user to channel (respects `+i` mode)
- `TOPIC` - View/change channel topic (respects `+t` mode)
- `MODE` - Channel modes: `i` (invite-only), `t` (topic lock), `k` (password), `o` (operator), `l` (user limit)

---

### Class Structure
```
Server
├── _port (int)                         // Port number for server
├── _server_fd (int)                    // Server listening socket fd
├── _password (string)                  // Server connection password
├── _pollfds (vector<pollfd>)           // All file descriptors being monitored
├── _clients (map<int, Client*>)        // fd → Client object mapping
└── _channels (map<string, Channel*>)   // channel name → Channel object mapping

Client
├── _fd (int)                           // Client socket file descriptor
├── _ipAddr (string)                    // Client IP address
├── _buffer (string)                    // Accumulates partial messages
├── _nickname (string)                  // User's nickname
├── _username (string)                  // User's username
├── _realname (string)                  // User's real name
├── _isRegistered (bool)                // Registration complete flag
└── _hasPassword (bool)                 // Password verified flag

Channel
├── _name (string)                      // Channel name (starts with #)
├── _members (vector<Client*>)          // All users in channel
├── _operators (vector<Client*>)        // Subset with operator privileges
├── _topic (string)                     // Channel topic (TOPIC command)
├── _password (string)                  // Channel key (MODE +k)
├── _inviteOnly (bool)                  // Invite-only flag (MODE +i)
├── _topicRestricted (bool)             // Topic restricted to ops (MODE +t)
├── _userLimit (int)                    // Max users, -1 = no limit (MODE +l)
└── _inviteList (vector<Client*>)       // Clients invited via INVITE command
```

### Data Flow
```
Network → poll() → recv() → Client buffer → extractLine() → parseMessage() → Command handlers
```

---

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

| Command | Status | Notes                                         |
| ------- | :----: | --------------------------------------------- |
| PASS    |   ✅   | Required before registration                  |
| NICK    |   ✅   | Nick collision handling, basic validation     |
| USER    |   ✅   | Registration flow                             |
| JOIN    |   ✅   | Enforces +i, +k, +l mode restrictions         |
| PART    |   ✅   | Multi-channel, optional reason                |
| PRIVMSG |   ✅   | Private messages and channel messages         |
| KICK    |   ✅   | Operator only, broadcasts to channel          |
| INVITE  |   ✅   | Operator only, adds to invite list            |
| TOPIC   |   ✅   | View and set; respects +t restriction         |
| MODE    |   ✅   | Supports i, t, k, o, l; multi-flag stacking   |
| PING    |   ✅   | PONG reply                                    |
| QUIT    |   ✅   | Cleanup and broadcast to all joined channels  |

Legend: ⬜ not done, 🟨 partial, ✅ done

---

## Technical Notes

### Basic Socket event loop

##### Server init
1. `socket()` → Create socket fd
2. `setsockopt()` → Set port to release after exit (faster testing)
3. `fcntl()` → Set non-blocking
4. `bind()` → Attach to port
5. `listen()` → Mark as passive (accepting connections)
6. `signal()` → Signal handlers (SIGINT, SIGQUIT, SIGPIPE)
7. Add server fd to poll array

##### Server run
1. `poll()` → Wait for events on any file descriptor
2. Check `revents & POLLIN` → Identify which fd has data ready
3. If fd is `_server_fd` (New Connection):
```
    accept()     → Create a new connection fd for the client
    fcntl()      → Set new client fd to O_NONBLOCK
    new Client() → Store client data (IP, fd) in _clients map
    push pollfd  → Add new fd to poll array
```
4. If fd is a client fd (Existing Connection):
```
    recv()          → Read incoming bytes into temporary buffer
    if recv <= 0    → closeClient(), remove from poll array
    appendBuffer()  → Append raw data to Client's internal buffer
    hasLine()       → Check if buffer contains '\n'
    extractLine()   → Extract complete message, remove from buffer
    parseMessage()  → Strip \r\n, extract command, route to handler
    check removal   → If command closed the client (QUIT), remove fd from poll array
```
### Message Parsing
```
┌─────────────────────────────────────────────────────────────┐
│ 1. Network Data Arrives                                     │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 2. poll() detects activity on client_fd                     │
│    - Checks: fds[i].revents & POLLIN                        │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ 3. recv(client_fd, buffer, 1024, 0)                         │
│    - Reads up to 1023 bytes from socket                     │
│    - Returns bytes_read                                     │
└─────────────────────────────────────────────────────────────┘
                            ↓
                    ┌───────────────┐
                    │ bytes_read?   │
                    └───────────────┘
                       /           \
                  <= 0              > 0
                     ↓               ↓
            ┌──────────────┐   ┌─────────────────────────────┐
            │ Client       │   │ 4. client->appendBuffer()   │
            │ Disconnected │   │    - Append to internal     │
            │              │   │      buffer string          │
            │ closeClient()│   └─────────────────────────────┘
            └──────────────┘              ↓
                            ┌─────────────────────────────────┐
                            │ 5. while (client->hasLine())    │
                            │    - Check if buffer contains   │
                            │      '\n' delimiter             │
                            └─────────────────────────────────┘
                                          ↓
                            ┌─────────────────────────────────┐
                            │ 6. msg = extractLine()          │
                            │    - Extract up to '\n'         │
                            │    - Remove from buffer         │
                            └─────────────────────────────────┘
                                          ↓
                            ┌─────────────────────────────────┐
                            │ 7. parseMessage(msg, fd)        │
                            │    - Strip '\r' and '\n'        │
                            │    - Extract command word       │
                            │    - Convert to uppercase       │
                            │    - Extract arguments          │
                            └─────────────────────────────────┘
                                          ↓
                            ┌─────────────────────────────────┐
                            │ 8. Route to command handler     │
                            │    PASS   → _executePASS()      │
                            │    NICK   → _executeNICK()      │
                            │    USER   → _executeUSER()      │
                            │    JOIN   → _executeJOIN()      │
                            │    PRIVMSG→ _executePRIVMSG()   │
                            │    etc.                         │
                            └─────────────────────────────────┘
```

### Client Authentication Flow (PASS, NICK, USER)
```
┌─────────────────────────────────────────────────────────────┐
│ Client Connects                                             │
│   hasPassword: false  nickname: ""  username: ""           │
│   isRegistered: false                                       │
└─────────────────────────────────────────────────────────────┘
                            ↓
              Commands can arrive in any order
                            ↓
       PASS              NICK                USER
        ↓                 ↓                   ↓
  Validate pw       Validate nick        Set username
  setHasPassword    setNickname          setRealname
                            ↓
         if (hasPassword && nickname != "" && username != "")
                            ↓
                  isRegistered = true
                  send RPL 001 Welcome
```

### State Model

- `_clients` map owns all `Client*` objects — freed in `closeClient()` and `~Server()`
- `_channels` map owns all `Channel*` objects — freed when empty or in `~Server()`
- When a client disconnects, they are removed from all channel member/operator/invite lists before deletion
- If a channel loses all operators, the first remaining member is promoted automatically
- Invite list (`_inviteList`) is per-channel; entries are cleared when the invited client successfully joins
---

## Testing

- see testing.md

---

## Resources

### IRC protocol references

* Chirc Tutorial - http://chi.cs.uchicago.edu/chirc/irc.html
* Modern IRC Client Protocol: A more readable guide to how modern clients interact with servers https://ircdocs.horse/specs/

* RFC 1459 — Internet Relay Chat Protocol (classic reference) https://tools.ietf.org/html/rfc1459
* RFC 2812 — Internet Relay Chat: Client Protocol https://tools.ietf.org/html/rfc2812
* RFC 2813 — Internet Relay Chat: Server Protocol (extra context)

* Beej's Guide to Network Programming: A classic reference for socket programming https://beej.us/guide/bgnet/

### Socket programming / multiplexing

* `man 2 socket`, `man 2 bind`, `man 2 listen`, `man 2 accept`, `man 2 recv`, `man 2 send`
* `man 2 poll` (or `man 2 select` if you use select)
* Beej’s Guide to Network Programming

### How AI was used

AI (Claude by Anthropic, ChatGPT ...etc) was used as a guided learning tool throughout this project.

* **INVITE command** — discussed design of invite list lifecycle (when to add, when to clear), error codes (341, 443, 473), and correct IRC message format for inviter vs target
* **MODE multi-flag parsing** — debugged the `needsParameter()` helper logic and parameter consumption loop for stacked flags like `+ikl`
* **JOIN mode enforcement** — verified correct placement of `+i`/`+k`/`+l` checks and `removeInvite()` call after successful join
* **Channel.cpp invite methods** — worked through `isInvited()`, `addInvite()`, `removeInvite()` design (const_iterator, pointer comparison, erase safety)
* **README** — generated and updated this document.

---

## Authors

* jngew🍌
* myuen🍌

---

## License

This project is part of a learning path and is not intended for use.
