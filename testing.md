# ft_irc Testing Checklist

---

# 1. Registration

## PASS

- [ ] Correct PASS allows registration
- [ ] Wrong PASS returns `464 Password incorrect`
- [ ] Wrong PASS closes connection

## NICK

- [ ] Valid nickname accepted
- [ ] Missing nickname returns `431`
- [ ] Invalid nickname returns `432`
- [ ] Duplicate nickname returns `433`

Test cases:
```
NICK
NICK #bob
NICK :bob
NICK bob 42
NICK 1bob
```

## USER

- [ ] USER sets username correctly
- [ ] USER + NICK + PASS triggers welcome message `001`
- [ ] Re-register attempt returns `462`

---

# 2. Basic Server Commands

## PING

- [ ] `PING test` returns `PONG test`

## Unknown command

- [ ] Unknown command returns `421`

Example:
```
RANDOM hello
```

---

# 3. JOIN

## Basic JOIN

- [ ] Create new channel
- [ ] First user becomes operator
- [ ] JOIN message broadcast
- [ ] `353` names list received
- [ ] `366` end of names

## JOIN existing channel

- [ ] Second user joins successfully
- [ ] All members see JOIN message

## Edge cases

- [ ] JOIN same channel twice does nothing
- [ ] JOIN without `#` normalized or rejected consistently
- [ ] JOIN with missing params returns `461`

Example:
```
JOIN
JOIN room
JOIN #room
JOIN #test wrongpass         # on +k channel
```

---

# 4. PRIVMSG

## User → User

- [ ] Message delivered to target user

## User → Channel

- [ ] Channel members receive message
- [ ] Sender does not receive own message (if excluded)

## Errors

- [ ] No recipient → `411`
- [ ] No text → `412`
- [ ] Invalid nick → `401`
- [ ] Non-member sending to channel → `404`

Test cases:
```
PRIVMSG
PRIVMSG bob
PRIVMSG #room
PRIVMSG bob :
PRIVMSG unknown :hello
```

---

# 5. PART

## Normal PART

- [ ] PART removes user from channel
- [ ] PART broadcast to members

## With reason

- [ ] Reason included in PART message

Example:
```
PART #room :bye
```


## Edge cases

- [ ] Missing parameter → `461`
- [ ] Nonexistent channel → `403`
- [ ] User not in channel → `442`

## Multiple channels

- [ ] `PART #a,#b,#c`
- [ ] Each channel processed independently

---

# 6. QUIT

## QUIT with reason

- [ ] QUIT message broadcast to channels
- [ ] User removed from all channels

## QUIT without reason

- [ ] Default reason used

## Channel cleanup

- [ ] Empty channel deleted

## Operator reassignment

- [ ] When operator quits, another member becomes operator

---

# 7. TOPIC

## View topic

- [ ] No topic → `331`
- [ ] Topic exists → `332`

## Set topic

- [ ] Topic updated
- [ ] Topic broadcast

Example:
```
TOPIC #room :hello world
```


## Topic restriction

With `+t` mode:

- [ ] Non-operator cannot change topic
- [ ] Operator can change topic

## Clear topic
```
TOPIC #room :
```

- [ ] Topic cleared successfully

---

# 8. MODE

Supported modes:
```
i invite-only
t topic restricted
k channel key
l user limit
o operator
```

## Query modes

- [ ] `MODE #room` returns `324`

## Permissions

- [ ] Non-op cannot change modes → `482`

## Mode operations

### Invite-only

- [ ] `MODE #room +i`
- [ ] `MODE #room -i`

### Topic restriction

- [ ] `MODE #room +t`
- [ ] `MODE #room -t`

### Channel key

- [ ] `MODE #room +k secret`
- [ ] `MODE #room -k`

### User limit

- [ ] `MODE #room +l 3`
- [ ] `MODE #room -l`

### Operator

- [ ] `MODE #room +o bob`
- [ ] `MODE #room -o bob`

## Mode errors

- [ ] Unknown mode → `472`
- [ ] Missing parameter → `461`

Examples:
```
MODE #test +i
MODE #test +k secret
MODE #test +l 3
MODE #test +t
MODE #test +o alice
MODE #test -i
MODE #test +ik secret      # multi-flag
```

## Invalid parameters

- [ ] `MODE #room +l abc`
- [ ] `MODE #room +l -1`
- [ ] `MODE #room +l 12abc`

---

# 9. KICK

Syntax:
```
KICK #channel user1,user2,user3 :reason
```

## Basic KICK

- [ ] Operator can kick member
- [ ] KICK message broadcast
- [ ] Target removed from channel

## No reason

- [ ] Default reason used

## Errors

- [ ] Missing parameters → `461`
- [ ] Nonexistent channel → `403`
- [ ] Kicker not in channel → `442`
- [ ] Kicker not operator → `482`
- [ ] Target not in channel → `441`
- [ ] Nick does not exist → `401`

## Multiple users

- [ ] `KICK #room bob,charlie :bye`
- [ ] Each user processed independently
- [ ] Invalid nick does not stop valid kicks

---

# 10. Channel Lifecycle

## Channel deletion

- [ ] Channel deleted when last user leaves

## Operator reassignment

- [ ] If last operator leaves → new operator assigned

Test cases:
```
QUIT
PART
KICK
```

---

# 11. Edge

## Extra spaces

- [ ] Commands parse correctly

Examples:
```
JOIN #room
MODE #room +k secret
KICK #room bob :bye
TOPIC #room :hello
```

## Blank lines

- [ ] Ignored safely

## Multiple commands in one packet

Example:
```
NICK a\r\nUSER a 0 * :A\r\nJOIN #x\r\n
```

- [ ] All commands processed correctly

## Fragmented input

Example TCP fragments:
```
PRIV
MSG #room :hello
```

- [ ] Buffered correctly

## Unregistered client sending commands
- [ ] Ignore?
```
PRIVMSG #test :hello        # before PASS/NICK/USER
```


## Partial data (nc specific)
- [ ] Type "JOI" then Ctrl+D, then "N #test" then Enter

## Client limit
- [ ] 3 clients try to join, first 2 succeed, third gets 471
```
MODE #test +l 2
```

---

# 12. Stability

Run under valgrind.

- [ ] No crashes
- [ ] No leaks
