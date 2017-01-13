import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

from queue import Empty
import unittest
import random
import time

from pytests.commands import *
from pytests.common import Client
from pytests.common import Server
from pytests.common import USERS_FILE, CWD


class TestMailServer(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.users = []
        file_path = os.path.join(CWD, "pytests", USERS_FILE)
        with open(file_path, 'r') as fd:
            for l in fd:
                line = l.replace('\n', '').split('\t')
                cls.users.append((line[0], line[1]))


        cls.mail_server = Server()
        cls.mail_server.start()

        clients_amount = random.randint(2, len(cls.users))
        cls.clients = []

        for _ in range(clients_amount):
            cls.clients.append(Client())

    @classmethod
    def tearDownClass(cls):
        cls.mail_server.stop()

    def setUp(self):
        if not self.mail_server:
            self.mail_server = Server()

        for i, client in enumerate(self.clients):
            if not client:
                self.clients[i] = Client()

    def tearDown(self):
        for client in self.clients:
            client.stop()

    def _get_user(self, index=0, name=None):
        if name is None:
            return self.users[index]

        for user in self.users:
            if name == user[0]:
                return user

        return None

    def test_a_multi_connect(self):
        for i, client in enumerate(self.clients):
            user = self._get_user(i)

            if not client.is_running():
                client.start()
                self._recv(client)

            self._send("User: %s\n" % user[0], client)
            self._send("Password: %s\n" % user[1], client)

            self.assertEqual(self._recv(client), CONNECTION_SUCCESSFUL_STR)

    def test_b_connect_bad_username(self):
        for i, client in enumerate(self.clients):
            user = self._get_user(i)

            if not client.is_running():
                client.start()
                self._recv(client)

            self._send("User: %sxx\n" % user[0], client)
            self._send("Password: %s\n" % user[1], client)

            self.assertEqual(self._recv(client), b"Failed on Login: Unknown user name.\n")

    def test_c_connect_some_bad_username_some_ok(self):
        for i, client in enumerate(self.clients):
            user = self._get_user(i)

            if not client.is_running():
                client.start()
                self._recv(client)

            if bool(random.getrandbits(1)):
                # This client will be bad connect
                self._send("User: %sxx\n" % user[0], client)
                self._send("Password: %s\n" % user[1], client)

                self.assertEqual(self._recv(client), b"Failed on Login: Unknown user name.\n")
            else:
                # This client will be good connect
                self._connect(client, user)

    def test_d_multi_compose(self):
        for i, client in enumerate(self.clients):
            self._connect(client, self._get_user(i))

            user1 = self._get_user(len(self.clients) - i - 1)
            to = "To: %s\n" % user1[0]
            subject = "Subject: Funny pictures\n"
            text = "Text: How are you? Long time no see!\n"

            self._send(COMPOSE_CMD, client)

            self._send(to, client)
            self._send(subject, client)
            self._send(text, client)

            self.assertEqual(self._recv(client), COMPOSE_REPONSE)

            to = "To: %s\n" % user1[0]
            subject = "Subject: Avengers Internship\n"
            text = "Text: Please send your CV to the new exiting Avengers Internship! #TonyStark\n"

            self._send(COMPOSE_CMD, client)

            self._send(to, client)
            self._send(subject, client)
            self._send(text, client)

            self.assertEqual(self._recv(client), COMPOSE_REPONSE)

    def test_e_multi_show_inbox(self):
        for i, client in enumerate(self.clients):
            sender_index = len(self.clients) - i - 1
            expected_mail1 = '1 %s "Funny pictures"' % self._get_user(sender_index)[0]
            expected_mail2 = '2 %s "Avengers Internship"' % self._get_user(sender_index)[0]

            self._connect(client, self._get_user(i))

            self._send(SHOW_INBOX_CMD, client)

            res = b""

            while True:
                try:
                    res += self._recv(client)
                except Empty:
                    break

            self.assertEqual(res, ("%s\n%s\n" % (expected_mail1, expected_mail2)).encode())

    def test_f_show_online_command(self):
        for i, client in enumerate(self.clients):
            self._connect(client, self._get_user(i))

        client = self.clients[0]

        self._send(SHOW_ONLINE_USERS_CMD, client)
        res = self._recv(client)

        self.assertEqual(res, (ONLINE_USERS_RESPONSE_FORMAT % ",".join([self._get_user(i)[0]
                                                                         for i in range(len(self.clients))])).encode())

    def test_g_msg_command(self):
        for i, client in enumerate(self.clients):
            self._connect(client, self._get_user(i))

        client0 = self.clients[0]
        client1 = self.clients[1]

        text = "Demo message."

        self._send(MSG_CMD_FORMAT % (self._get_user(1)[0], text), client0)
        res = self._recv(client1)

        self.assertEqual(res, (MSG_RESPONSE_FORMAT % (self._get_user(0)[0], text)).encode())

    def test_h_msg_command_to_many_users(self):
        for i, client in enumerate(self.clients):
            self._connect(client, self._get_user(i))

        client0 = self.clients[0]

        text = "Demo message %d."

        for i in range(1, len(self.clients)):
            self._send(MSG_CMD_FORMAT % (self._get_user(i)[0], text % i), client0)
            res = self._recv(self.clients[i])

            self.assertEqual(res, (MSG_RESPONSE_FORMAT % (self._get_user(0)[0], text % i)).encode())

    def test_i_msg_offline_user(self):
        sender = self.clients[0]
        receiver = self.clients[1]

        sender_user = self._get_user(0)
        receiver_user = self._get_user(1)

        self._connect(sender, sender_user)

        text = "Demo message."

        self._send(MSG_CMD_FORMAT % (receiver_user[0], text), sender)

        self._connect(receiver, receiver_user)
        self._send(SHOW_INBOX_CMD, receiver)
        res = self._recv(receiver)

        self.assertEqual(res, ('1 %s "%s"\n' % (sender_user[0], OFFLINE_MSG_TITLE)).encode())

        self._send(GET_MAIL_CMD % 1, receiver)
        res = b""
        while True:
            try:
                res += self._recv(receiver)
            except Empty:
                break

        self.assertEqual(res, (RECV_MAIL_FORMAT % (sender_user[0],
                                                   receiver_user[0],
                                                   OFFLINE_MSG_TITLE,
                                                   text)).encode('utf-8'))

    def test_j_msg_command_when_connected_and_then_disconnected(self):
        sender = self.clients[0]
        receiver = self.clients[1]

        sender_user = self._get_user(0)
        receiver_user = self._get_user(1)

        text1 = "Demo message 1."
        text2 = "Demo message 2."

        self._connect(sender, sender_user)
        self._connect(receiver, receiver_user)

        self._send(MSG_CMD_FORMAT % (receiver_user[0], text1), sender)
        res = self._recv(receiver)

        self.assertEqual(res, (MSG_RESPONSE_FORMAT % (sender_user[0], text1)).encode())

        # Client 1 disconnects
        receiver.stop()
        time.sleep(4)

        self._send(MSG_CMD_FORMAT % (receiver_user[0], text2), sender)

        self._connect(receiver, receiver_user)
        self._send(SHOW_INBOX_CMD, receiver)
        res = self._recv(receiver)

        self.assertEqual(res, ('1 %s "%s"\n' % (sender_user[0], OFFLINE_MSG_TITLE)).encode())

        self._send(GET_MAIL_CMD % 1, receiver)
        res = b""
        while True:
            try:
                res += self._recv(receiver)
            except Empty:
                break

        self.assertEqual(res, (RECV_MAIL_FORMAT % (sender_user[0],
                                                   receiver_user[0],
                                                   OFFLINE_MSG_TITLE,
                                                   text2)).encode('utf-8'))

    def test_k_ongoing_chat(self):
        c0 = self.clients[0]
        c1 = self.clients[1]

        u0 = self._get_user(0)
        u1 = self._get_user(1)

        msgs0 = ["Hey!", "All good mate", "Yes, indeed", "No, thank you kind sir", "=)", "Bye!"]
        msgs1 = ["Whats'uppp??", "Cool dude!", "Wanna get wasted??!", "K", "Bye!", "^_0"]

        self._connect(c0, u0)
        self._connect(c1, u1)

        for i in range(len(msgs0)):
            self._send(MSG_CMD_FORMAT % (u1[0], msgs0[i]), c0)
            res = self._recv(c1)
            self.assertEqual(res, (MSG_RESPONSE_FORMAT % (u0[0], msgs0[i])).encode())

            self._send(MSG_CMD_FORMAT % (u0[0], msgs1[i]), c1)
            res = self._recv(c0)
            self.assertEqual(res, (MSG_RESPONSE_FORMAT % (u1[0], msgs1[i])).encode())


    # --- PRIVATE --- #

    def _connect(self, client, user=None):
        if client.is_running():
            client.stop()

        client.start()

        res = client.recv()

        assert res == WELCOME_MSG, "%s" % res

        if user is not None:
            client.send("User: %s\n" % user[0])
            client.send("Password: %s\n" % user[1])

            res = self._recv(client)

            assert res == CONNECTION_SUCCESSFUL_STR, "%s" % res


    def _send(self, cmd, client):
        return client.send(cmd)

    def _recv(self, client):
        return client.recv()


if __name__ == "__main__":
    unittest.main()
