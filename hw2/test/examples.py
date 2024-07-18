#!/usr/bin/env python3
from pwn import *

SERVER = "localhost"
PORT = 8080


def get_connection():
    conn = remote(SERVER, PORT)
    conn.recvuntil(b"Welcome to the Chat server.")
    conn.recvuntil(b"*********************************\n")
    return conn


def command(conn, command, expected_output=None, prompt=True):
    if prompt:
        conn.recvuntil(b"% ")

    # For testcase, command can't be empty
    if len(command) > 0:
        conn.sendline(command)

    if expected_output:
        for output in expected_output:
            line = conn.recvline().strip()
            assert line == output, f"Expected: {output}\nActual: {line}"


def example1():
    ta1 = get_connection()
    ta3 = get_connection()

    command(ta1, b"register", [b"Usage: register <username> <password>"])
    command(ta1, b"register ta1 420420", [b"Register successfully."])
    command(ta1, b"register ta1 420420", [b"Username is already used."])
    command(ta1, b"login", [b"Usage: login <username> <password>"])
    command(ta1, b"login ta1", [b"Usage: login <username> <password>"])
    command(ta1, b"login ta1 000000", [b"Login failed."])
    command(ta1, b"login Tom 420420", [b"Login failed."])
    command(ta1, b"login ta1 420420", [b"Welcome, ta1."])
    command(ta1, b"whoami", [b"ta1"])
    command(ta1, b"login ta1 420420", [b"Please logout first."])
    command(ta1, b"logout", [b"Bye, ta1."])
    command(ta1, b"logout", [b"Please login first."])
    command(ta1, b"register ta2 ta2", [b"Register successfully."])

    command(ta3, b"register ta3 ta3", [b"Register successfully."])
    command(ta3, b"login ta3 ta3", [b"Welcome, ta3."])
    command(ta3, b"list-user", [b"ta1 offline", b"ta2 offline", b"ta3 online"])

    command(ta1, b"exit")

    command(ta3, b"set-status happyhappy", [b"set-status failed"])
    command(ta3, b"set-status busy", [b"ta3 busy"])
    command(ta3, b"list-user", [b"ta1 offline", b"ta2 offline", b"ta3 busy"])
    command(ta3, b"exit", [b"Bye, ta3."])

    ta1.close()
    ta3.close()


def example2():
    ta1 = get_connection()
    ta2 = get_connection()

    command(ta1, b"login ta1 420420", [b"Welcome, ta1."])
    command(ta1, b"enter-chat-room", [b"Usage: enter-chat-room <number>"])
    command(ta1, b"enter-chat-room 1010", [b"Number 1010 is not valid."])
    command(
        ta1,
        b"enter-chat-room 20",
        [b"Welcome to the public chat room.", b"Room number: 20", b"Owner: ta1"],
    )

    command(ta1, b"/exit-chat-room", prompt=False)

    command(
        ta1,
        b"enter-chat-room 30",
        [b"Welcome to the public chat room.", b"Room number: 30", b"Owner: ta1"],
    )

    command(ta1, b"hello, i am ta1.", [b"[ta1]: *****, i am ta1."], prompt=False)
    command(
        ta1,
        b"i am waiting for everyone.",
        [b"[ta1]: i am waiting for everyone."],
        prompt=False,
    )

    command(ta2, b"login ta2 ta2", [b"Welcome, ta2."])
    command(ta2, b"list-chat-room", [b"ta1 20", b"ta1 30"])
    command(ta2, b"close-chat-room", [b"Usage: close-chat-room <number>"])
    command(ta2, b"close-chat-room 30", [b"Only the owner can close this chat room."])

    command(
        ta2,
        b"enter-chat-room 30",
        [
            b"Welcome to the public chat room.",
            b"Room number: 30",
            b"Owner: ta1",
            b"[ta1]: *****, i am ta1.",
            b"[ta1]: i am waiting for everyone.",
        ],
    )
    command(ta1, b"", [b"ta2 had enter the chat room."], prompt=False)

    command(ta2, b"merry christmas!", [b"[ta2]: merry christmas!"], prompt=False)
    command(ta1, b"", [b"[ta2]: merry christmas!"], prompt=False)

    command(ta1, b"exit", [b"[ta1]: exit"], prompt=False)
    command(ta2, b"", [b"[ta1]: exit"], prompt=False)

    command(ta2, b"/close-chat-room 30", [b"Error: Unknown command"], prompt=False)

    command(ta1, b"/exit-chat-room", prompt=False)
    command(ta2, b"", [b"ta1 had left the chat room."], prompt=False)

    command(ta1, b"list-chat-room", [b"ta1 20", b"ta1 30"])
    command(ta1, b"close-chat-room 35", [b"Chat room 35 does not exist."])

    command(ta1, b"close-chat-room 20", [b"Chat room 20 was closed."])
    command(ta1, b"list-chat-room", [b"ta1 30"])

    command(ta1, b"close-chat-room 30", [b"Chat room 30 was closed."])
    command(ta2, b"", [b"Chat room 30 was closed."], prompt=False)

    command(ta1, b"list-chat-room")
    command(ta1, b"exit", [b"Bye, ta1."])

    command(ta2, b"close-chat-room 30", [b"Chat room 30 does not exist."])
    command(ta2, b"hello", [b"Error: Unknown command"])
    command(ta2, b"exit", [b"Bye, ta2."])

    ta1.close()
    ta2.close()


def example3():
    bob = get_connection()
    tom = get_connection()
    nobody = get_connection()

    command(bob, b"register Bob 55555", [b"Register successfully."])
    command(bob, b"login Bob 55555", [b"Welcome, Bob."])
    command(
        bob,
        b"enter-chat-room 25",
        [
            b"Welcome to the public chat room.",
            b"Room number: 25",
            b"Owner: Bob",
        ],
    )

    command(bob, b"I will win !!!", [b"[Bob]: I will win !!!"], prompt=False)
    command(bob, b"/delete-pin", [b"No pin message in chat room 25"], prompt=False)
    command(
        bob,
        b"/pin You are the challenger.",
        [b"Pin -> [Bob]: You are the challenger."],
        prompt=False,
    )
    command(bob, b"/exit-chat-room", prompt=False)

    command(tom, b"register Tom 22222", [b"Register successfully."])
    command(tom, b"login Tom 22222", [b"Welcome, Tom."])
    command(
        tom,
        b"enter-chat-room 25",
        [
            b"Welcome to the public chat room.",
            b"Room number: 25",
            b"Owner: Bob",
            b"[Bob]: I will win !!!",
            b"Pin -> [Bob]: You are the challenger.",
        ],
    )

    command(
        bob,
        b"enter-chat-room 25",
        [
            b"Welcome to the public chat room.",
            b"Room number: 25",
            b"Owner: Bob",
            b"[Bob]: I will win !!!",
            b"Pin -> [Bob]: You are the challenger.",
        ],
    )
    command(tom, b"", [b"Bob had enter the chat room."], prompt=False)

    command(tom, b"hello", [b"[Tom]: *****"], prompt=False)
    command(bob, b"", [b"[Tom]: *****"], prompt=False)

    command(bob, b"?", [b"[Bob]: ?"], prompt=False)
    command(tom, b"", [b"[Bob]: ?"], prompt=False)

    command(bob, b"domain expansion.", [b"[Bob]: ****************."], prompt=False)
    command(tom, b"", [b"[Bob]: ****************."], prompt=False)

    command(tom, b"What?", [b"[Tom]: What?"], prompt=False)
    command(bob, b"", [b"[Tom]: What?"], prompt=False)

    command(
        bob, b"domain expansion!!!???", [b"[Bob]: ****************!!!???"], prompt=False
    )
    command(tom, b"", [b"[Bob]: ****************!!!???"], prompt=False)

    command(
        tom,
        b"/pin You are an ordinary person.",
        [b"Pin -> [Tom]: You are an ordinary person."],
        prompt=False,
    )
    command(bob, b"", [b"Pin -> [Tom]: You are an ordinary person."], prompt=False)

    command(bob, b"/exit-chat-room", prompt=False)
    command(tom, b"", [b"Bob had left the chat room."], prompt=False)

    command(bob, b"set-status offline", [b"Bob offline"])
    command(
        bob,
        b"enter-chat-room 25",
        [
            b"Welcome to the public chat room.",
            b"Room number: 25",
            b"Owner: Bob",
            b"[Bob]: I will win !!!",
            b"[Tom]: *****",
            b"[Bob]: ?",
            b"[Bob]: ****************.",
            b"[Tom]: What?",
            b"[Bob]: ****************!!!???",
            b"Pin -> [Tom]: You are an ordinary person.",
        ],
    )
    command(tom, b"", [b"Bob had enter the chat room."], prompt=False)

    command(bob, b"I'm sorry.", [b"[Bob]: I'm sorry."], prompt=False)
    command(tom, b"", [b"[Bob]: I'm sorry."], prompt=False)

    command(
        bob,
        b"I couldn't bring out the best in you.",
        [b"[Bob]: I couldn't bring out the best in you."],
        prompt=False,
    )
    command(tom, b"", [b"[Bob]: I couldn't bring out the best in you."], prompt=False)

    command(tom, b"/list-user", [b"Bob offline", b"Tom online"], prompt=False)

    command(bob, b"/delete-pin", prompt=False)

    command(tom, b"I won't forget you.", [b"[Tom]: I won't forget you."], prompt=False)
    command(bob, b"", [b"[Tom]: I won't forget you."], prompt=False)

    command(tom, b"You can't beat me.", [b"[Tom]: You can't beat me."], prompt=False)
    command(bob, b"", [b"[Tom]: You can't beat me."], prompt=False)

    command(tom, b"Cheer up !!!", [b"[Tom]: Cheer up !!!"], prompt=False)
    command(bob, b"", [b"[Tom]: Cheer up !!!"], prompt=False)

    command(bob, b"/exit-chat-room", prompt=False)
    command(tom, b"", [b"Bob had left the chat room."], prompt=False)

    command(tom, b"Bye Bye.", [b"[Tom]: Bye Bye."], prompt=False)
    command(tom, b"Who else?", [b"[Tom]: Who else?"], prompt=False)

    command(nobody, b"register nobody 11111", [b"Register successfully."])
    command(nobody, b"login nobody 11111", [b"Welcome, nobody."])
    command(
        nobody,
        b"enter-chat-room 25",
        [
            b"Welcome to the public chat room.",
            b"Room number: 25",
            b"Owner: Bob",
            b"[Bob]: ****************.",
            b"[Tom]: What?",
            b"[Bob]: ****************!!!???",
            b"[Bob]: I'm sorry.",
            b"[Bob]: I couldn't bring out the best in you.",
            b"[Tom]: I won't forget you.",
            b"[Tom]: You can't beat me.",
            b"[Tom]: Cheer up !!!",
            b"[Tom]: Bye Bye.",
            b"[Tom]: Who else?",
        ],
    )
    command(tom, b"", [b"nobody had enter the chat room."], prompt=False)

    command(nobody, b"/exit-chat-room", prompt=False)
    command(tom, b"", [b"nobody had left the chat room."], prompt=False)

    command(tom, b"/exit-chat-room", prompt=False)

    # For me to play
    command(bob, b"exit", [b"Bye, Bob."])
    command(tom, b"exit", [b"Bye, Tom."])
    command(nobody, b"exit", [b"Bye, nobody."])

    bob.close()
    tom.close()
    nobody.close()


if __name__ == "__main__":
    if len(sys.argv) > 1:
        PORT = int(sys.argv[1])

    server = process(["./hw2_chat_server", str(PORT)])

    example1()
    example2()
    example3()

    server.interactive()
    server.close()


"""
### Example 1

> `register` `login` `logout` `whoami` `set-status` `list-user`

- ta1

```shell
    bash$ nc localhost 1234
    *********************************
    ** Welcome to the Chat server. **
    *********************************
1.  % register
    Usage: register <username> <password>
2.  % register ta1 420420
    Register successfully.
3.  % register ta1 420420
    Username is already used.
4.  % login
    Usage: login <username> <password>
5.  % login ta1
    Usage: login <username> <password>
6.  % login ta1 000000
    Login failed.
7.  % login Tom 420420
    Login failed.
8.  % login ta1 420420
    Welcome, ta1.
9.  % whoami
    ta1
10. % login ta1 420420
    Please logout first.
11. % logout
    Bye, ta1.
12. % logout
    Please login first.
13. % register ta2 777777
    Register successfully.
17. % exit
```

- ta3 (login after ta2 has registered)

```shell
    bash$ nc localhost 1234
    *********************************
    ** Welcome to the Chat server. **
    *********************************
14. % register ta3 ta3
    Register successfully.
15. % login ta3 ta3
    Welcome, ta3.
16. % list-user
    ta1 offline
    ta2 offline
    ta3 online
18. % set-status happyhappy
    set-status failed
19. % set-status busy
    ta3 busy
20. % list-user
    ta1 offline
    ta2 offline
    ta3 busy
21. % exit
    Bye, ta3.
```

### Example 2

> `enter-chat-room <number>` `list-chat-room` `close-chat-room <number>` `unknown command` `chat`

- ta1 (login as a registered user ta1)

```shell
    *********************************
    ** Welcome to the Chat server. **
    *********************************
1.  % login ta1 420420
    Welcome, ta1.
2.  % enter-chat-room
    Usage: enter-chat-room <number>
3.  % enter-chat-room 1010
    Number 1010 is not valid.
4.  % enter-chat-room 20
    Welcome to the public chat room.
    Room number: 20
    Owner: ta1
5.  /exit-chat-room
6.  % enter-chat-room 30
    Welcome to the public chat room.
    Room number: 30
    Owner: ta1
7.  hello, i am ta1.
    [ta1]: *****, i am ta1.
8.  i am waiting for everyone.
    [ta1]: i am waiting for everyone.
    ta2 had enter the chat room.
    [ta2]: merry christmas!
15. exit
    [ta1]: exit
17. /exit-chat-room
18. % list-chat-room
    ta1 20
    ta1 30
19. % close-chat-room 35
    Chat room 35 does not exist.
20. % close-chat-room 20
    Chat room 20 was closed.
21. % list-chat-room
    ta1 30
22. % close-chat-room 30
    Chat room 30 was closed.
23. % list-chat-room
24. % exit
    Bye, ta1.
```

- ta2 (login as a registered user ta2)

```shell
    *********************************
    ** Welcome to the Chat server. **
    *********************************
9.  % login ta2 ta2
    Welcome, ta2.
10. % list-chat-room
    ta1 20
    ta1 30
11. % close-chat-room
    Usage: close-chat-room <number>
12. % close-chat-room 30
    Only the owner can close this chat room.
13. % enter-chat-room 30
    Welcome to the public chat room.
    Room number: 30
    Owner: ta1
    [ta1]: *****, i am ta1.
    [ta1]: i am waiting for everyone.
14. merry christmas!
    [ta2]: merry christmas!
    [ta1]: exit
16. /close-chat-room 30
    Error: Unknown command
    ta1 had left the chat room.
    Chat room 30 was closed.
25. % close-chat-room 30
    Chat room 30 does not exist.
26. % hello
    Error: Unknown command
27. % exit
    Bye, ta2.
```

### Example 3

> `/pin <message>` `/delete-pin` `/exit-chat-room` `/list-user` `chat`

- Bob (The second `enter-chat-room` command is sent after Tom enters the chat room)

```shell
    *********************************
    ** Welcome to the Chat server. **
    *********************************
1.  % register Bob 55555
    Register successfully.
2.  % login Bob 55555
    Welcome, Bob. 
3.  % enter-chat-room 25
    Welcome to the public chat room.
    Room number: 25
    Owner: Bob
4.  I will win !!!
    [Bob]: I will win !!!
5.  /delete-pin
    No pin message in chat room 25
6.  /pin You are the challenger.
    Pin -> [Bob]: You are the challenger.
7.  /exit-chat-room   
11. % enter-chat-room 25
    Welcome to the public chat room.
    Room number: 25
    Owner: Bob
    [Bob]: I will win !!!
    Pin -> [Bob]: You are the challenger.
    [Tom]: *****
13. ?
    [Bob]: ?
14. domain expansion.
    [Bob]: ****************.
    [Tom]: What?
16. domain expansion!!!???
    [Bob]: ****************!!!???
    Pin -> [Tom]: You are an ordinary person.
18. /exit-chat-room
19. % set-status offline
    Bob offline
20. % enter-chat-room 25
    Welcome to the public chat room.
    Room number: 25
    Owner: Bob
    [Bob]: I will win !!!
    [Tom]: *****
    [Bob]: ?
    [Bob]: ****************.
    [Tom]: What?
    [Bob]: ****************!!!???
    Pin -> [Tom]: You are an ordinary person.
21. I'm sorry.
    [Bob]: I'm sorry.
22. I couldn't bring out the best in you.
    [Bob]: I couldn't bring out the best in you.
24. /delete-pin
    [Tom]: I won't forget you.
    [Tom]: You can't beat me.
    [Tom]: Cheer up !!!
28. /exit-chat-room
% 
```

- Tom (enter the chat room after the first `exit-chat-room` command of Bob)

```shell
    *********************************
    ** Welcome to the Chat server. **
    *********************************
8.  % register Tom 22222
    Register successfully.
9.  % login Tom 22222
    Welcome, Tom.
10. % enter-chat-room 25
    Welcome to the public chat room.
    Room number: 25
    Owner: Bob
    [Bob]: I will win !!!
    Pin -> [Bob]: You are the challenger.
    Bob had enter the chat room.
12. hello
    [Tom]: *****
    [Bob]: ?
    [Bob]: ****************.
15. What?
    [Tom]: What?
    [Bob]: ****************!!!???
17. /pin You are an ordinary person.
    Pin -> [Tom]: You are an ordinary person.
    Bob had left the chat room.
    Bob had enter the chat room.
    [Bob]: I'm sorry.
    [Bob]: I couldn't bring out the best in you.
23. /list-user
    Bob offline
    Tom online
25. I won't forget you.
    [Tom]: I won't forget you.
26. You can't beat me.
    [Tom]: You can't beat me.
27. Cheer up !!!
    [Tom]: Cheer up !!!
    Bob had left the chat room.
29. Bye Bye.
    [Tom]: Bye Bye.
30. Who else?
    [Tom]: Who else? 
    nobody had enter the chat room.
    nobody had left the chat room.
35. /exit-chat-room
% 
```

- nobody (enter the chat room after the third `exit-chat-room` command of Bob)

```bash
    *********************************
    ** Welcome to the Chat server. **
    *********************************
31. % register nobody 11111
    Register successfully.
32. % login nobody 11111
    Welcome, nobody.
33. % enter-chat-room 25
    Welcome to the public chat room.
    Room number: 25
    Owner: Bob
    [Bob]: ****************.
    [Tom]: What?
    [Bob]: ****************!!!???
    [Bob]: I'm sorry.
    [Bob]: I couldn't bring out the best in you.
    [Tom]: I won't forget you.
    [Tom]: You can't beat me.
    [Tom]: Cheer up !!!
    [Tom]: Bye Bye.
    [Tom]: Who else?
34. /exit-chat-room
% 
```
"""
