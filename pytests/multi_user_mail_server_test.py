import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

from queue import Empty
import unittest
import random

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

        clients_amount = random.randint(1, len(cls.users))
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

    def test_a_a_connect_bad_username(self):
        for i, client in enumerate(self.clients):
            user = self._get_user(i)

            if not client.is_running():
                client.start()
                self._recv(client)

            self._send("User: %sxx\n" % user[0], client)
            self._send("Password: %s\n" % user[1], client)

            self.assertEqual(self._recv(client), b"Failed on Login: Unknown user name.\n")

    def test_a_b_connect_some_bad_username_some_ok(self):
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

    def test_b_multi_compose(self):
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

    def test_c_multi_show_inbox(self):
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
