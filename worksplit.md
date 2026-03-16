## How we Split the ft_IRC Project

### Core Idea
Treat the **socket like a file stream**.

The networking layer reads data from clients and sends data back.  
The IRC layer only processes **strings representing IRC commands**.

---

# A : Network / Server Infrastructure

focuses on **connections and message flow**.

handle the low-level networking and convert socket data into full IRC command strings.

## Responsibilities

### Server lifecycle
- socket creation
- bind
- listen
- accept new clients

### Poll / event loop
Handling multiple clients using:

poll()
recv()
disconnect handling

Example functions:

init()
run()

These functions manage sockets and the polling system.

---

### Client connection management

Responsibilities:
- storing connected clients
- removing disconnected clients
- cleanup

Example function:

closeClient()

This also removes the user from channels and invite lists.

---

### Message buffering

Clients send partial messages.  
The server must accumulate them until a full line arrives.

This is handled inside the `Client` class.

Functions:

appendBuffer()
hasLine()
extractLine()

These convert the socket stream into **complete IRC messages**.

So the network layer produces messages like:

"NICK alice"  
"JOIN #room"  
"PRIVMSG #room :hello"

Once a full line is ready, it is passed to the command parser.

---

# B : IRC Protocol / Command Logic

focuses on **IRC rules and command behaviour**.

do **not worry about sockets, poll, or networking**.

only implement command logic.

## Command behaviour functions

_executeJOIN()  
_executePART()  
_executeKICK()  
_executeMODE()  
_executeTOPIC()  
_executePRIVMSG()  
_executeINVITE()

---

### Command routing

The router function calls these command handlers.

parseMessage(...)

This extracts the command and arguments.

So the IRC developer receives:
- a client pointer
- an argument string

Example:

_executeJOIN(client, "#room")

---

### Tasks for the IRC logic developer

- update channels
- check permissions
- broadcast correct messages
- send correct replies
- enforce IRC rules

---

# Shared Classes

Both developers interact with the same core classes.

## Client

Represents a connected user.

Examples:

getNickname()  
getPrefix()  
getFd()

---

## Channel

Represents an IRC channel.

Examples:

addMember()  
removeMember()  
broadcast()  
isOperator()  
setTopic()

Channels manage:
- membership
- operators
- topics
- invite lists
- channel modes

---

# Code Organisation Note

A lot of our code currently lives inside **Server.cpp**, but that is simply a design choice and **not ideal organisation**.

Conceptually, the project still has two layers:

Network layer  
IRC protocol layer


---

# Interface Agreement

Need to agree on **function interfaces**.

Once these are fixed, both can work safely without constantly changing each other's code.

---

## Server → IRC command interface

void _executeJOIN(Client*, std::string arg);  
void _executeKICK(Client*, std::string arg);

---

## Channel interface

addMember(Client*)  
removeMember(Client*)  
isOperator(Client*)  
broadcast(std::string message, Client* exclude)

---
