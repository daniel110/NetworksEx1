import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

import subprocess
import socket
import struct
from queue import Empty
import unittest
import time

from pytests.commands import *
from pytests.common import Client
from pytests.common import Server
from pytests.common import USERS_FILE, SERVER_PORT, CWD


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

        cls.client = Client()

    @classmethod
    def tearDownClass(cls):
        cls.mail_server.stop()

    def setUp(self):
        if not self.mail_server:
            self.mail_server = Server()

        if not self.client:
            self.client = Client()

    def tearDown(self):
        self.client.stop()

    def _get_user(self, index=0, name=None):
        if name is None:
            return self.users[index]

        for user in self.users:
            if name == user[0]:
                return user

        return None

    def test_a_connect(self):
        user = self._get_user(0)

        if not self.client.is_running():
            self.client.start()
            self._recv()

        self._send("User: %s\n" % user[0])
        self._send("Password: %s\n" % user[1])

        self.assertEqual(self._recv(), CONNECTION_SUCCESSFUL_STR)

    def test_a_a_connect_bad_username(self):
        user = self._get_user(0)

        if not self.client.is_running():
            self.client.start()
            self._recv()

        self._send("User: %sxx\n" % user[0])
        self._send("Password: %s\n" % user[1])

        self.assertEqual(self._recv(), b"Failed on Login: Unknown user name.\n")

    def test_a_a_connect_bad_password(self):
        user = self._get_user(0)

        if not self.client.is_running():
            self.client.start()
            self._recv()

        self._send("User: %s\n" % user[0])
        self._send("Password: %sxx\n" % user[1])

        self.assertEqual(self._recv(), b"Failed on Login: Wrong password.\n")

    def test_b_compose(self):
        self._connect(self._get_user(0))

        user1 = self._get_user(1)
        to = "To: %s\n" % user1[0]
        subject = "Subject: Funny pictures\n"
        text = "Text: How are you? Long time no see!\n"

        self._send(COMPOSE_CMD)

        self._send(to)
        self._send(subject)
        self._send(text)

        self.assertEqual(self._recv(), COMPOSE_REPONSE)

        to = "To: %s\n" % user1[0]
        subject = "Subject: Avengers Internship\n"
        text = "Text: Please send your CV to the new exiting Avengers Internship! #TonyStark\n"

        self._send(COMPOSE_CMD)

        self._send(to)
        self._send(subject)
        self._send(text)

        self.assertEqual(self._recv(), COMPOSE_REPONSE)

    def test_c_show_inbox(self):
        expected_mail1 = '1 %s "Funny pictures"' % self._get_user(0)[0]
        expected_mail2 = '2 %s "Avengers Internship"' % self._get_user(0)[0]

        self._connect(self._get_user(1))

        self._send(SHOW_INBOX_CMD)

        res = b""

        while True:
            try:
                res += self._recv()
            except Empty:
                break

        self.assertEqual(res, ("%s\n%s\n" % (expected_mail1, expected_mail2)).encode())

    def test_d_get_mail(self):
        user0 = self._get_user(0)
        user1 = self._get_user(1)

        frm = user0[0]
        to = user1[0]
        subject = "Avengers Internship"
        text = "Please send your CV to the new exiting Avengers Internship! #TonyStark"

        self._connect(user1)

        self._send(GET_MAIL_CMD % 2)

        res = b""

        while True:
            try:
                res += self._recv()
            except Empty:
                break

        self.assertEqual(res, (RECV_MAIL_FORMAT % (frm, to, subject, text)).encode('utf-8'))

        self._send("GET_MAIL                2\n")  # Server should still be able to parse this

        res = b""

        while True:
            try:
                res += self._recv()
            except Empty:
                break

        self.assertEqual(res, (RECV_MAIL_FORMAT % (frm, to, subject, text)).encode('utf-8'))

    def test_e_delete_mail(self):
        expected_mail2 = '2 %s "Avengers Internship"\n' % self._get_user(0)[0]

        self._connect(self._get_user(1))

        self._send(DELETE_MAIL_CMD % 1)

        self._send(SHOW_INBOX_CMD)

        self.assertEqual(self._recv(), expected_mail2.encode('utf-8'))

    def test_f_quit(self):
        # TODO: How to test quit?
        self.client.start()

        time.sleep(1)

        pid = self.client.pid

        p1 = subprocess.Popen(['ps', '-fade'], stdout=subprocess.PIPE)
        p2 = subprocess.Popen(['grep', str(pid)], stdin=p1.stdout, stdout=subprocess.PIPE)
        res = p2.communicate()[0].split(b'\n')

        self.assertGreater(res[0].decode().index('./mail_client'), 0)


        self.client.mail_client.stdin.write(QUIT_CMD.encode('utf-8'))
        self.client.mail_client.stdin.flush()

        time.sleep(2)

        p1 = subprocess.Popen(['ps', '-fade'], stdout=subprocess.PIPE)
        p2 = subprocess.Popen(['grep', str(pid)] , stdin=p1.stdout, stdout=subprocess.PIPE)
        res = p2.communicate()[0].split(b'\n')

        self.assertRaises(ValueError, res[0].decode().index, './mail_client')  # should not be found

    # --- TEST BAD STUFF --- #

    def test_g_bad_login_user_prefix(self):
        user = self._get_user(0)

        if not self.client.is_running():
            self.client.start()
            self._recv()

        self._send("NotUser: %s\n" % user[0])

        self.assertEqual(self._recv(), b"Failed on Login: Unmatched Prefix, Expected: User:\n")

    def test_h_bad_login_password_prefix(self):
        user = self._get_user(0)

        if not self.client.is_running():
            self.client.start()
            self._recv()

        self._send("User: %s\n" % user[0])
        self._send("!Password: %s\n" % user[1])

        self.assertEqual(self._recv(), b"Failed on Login: Unmatched Prefix, Expected: Password:\n")

    def test_i_bad_get_mail_id(self):
        user1 = self._get_user(1)

        self._connect(user1)

        self._send(GET_MAIL_CMD % 100)  # Not suppose to have mail with id 100
        self.assertEqual(self._recv(), b"Failed on Get Mail: Unknown mail id.\n")

        self._send("GET_MAIL not_a_number\n")
        self.assertEqual(self._recv(), b"Unable to extract mail id.\n")

        self._send("GET_MAIL 2 2\n")
        self.assertEqual(self._recv(), b"The first and last parameter of GET_MAIL should be int\n")

        self._send("GET_MAIL 2.2\n")
        self.assertEqual(self._recv(), b"The first and last parameter of GET_MAIL should be int\n")

    def test_j_bad_delete_mail_id(self):
        user1 = self._get_user(1)

        self._connect(user1)

        self._send(DELETE_MAIL_CMD % 100)  # Not suppose to have mail with id 100
        self.assertEqual(self._recv(), b"Failed on delete mail: Unknown mail id.\n")

        self._send("DELETE_MAIL not_a_number\n")
        self.assertEqual(self._recv(), b"Unable to extract mail id.\n")

        self._send("DELETE_MAIL 2 2\n")
        self.assertEqual(self._recv(),
                         b"The first and last parameter of DELETE_MAIL should be int\n")

        self._send("DELETE_MAIL 2.2\n")
        self.assertEqual(self._recv(),
                         b"The first and last parameter of DELETE_MAIL should be int\n")

    def test_k_extra_args(self):
        self._connect(self._get_user(1))

        self._send("SHOW_INBOX 1\n")
        self.assertEqual(self._recv(), b"Got Extra arguments. Try command again.\n")

    def test_l_bad_commands(self):
        self._connect(self._get_user(1))

        self._send("NOT_A_VALID_CMD\n")
        self.assertEqual(self._recv(), b"Invalid command type name\n")

        self._send("SHOW_INBOX1\n")
        self.assertEqual(self._recv(), b"Invalid command type name\n")

        self._send("SHOW INBOX\n")
        self.assertEqual(self._recv(), b"Invalid command type name\n")

        self._send("1 SHOW_INBOX\n")
        self.assertEqual(self._recv(), b"Invalid command type name\n")

    def test_m_bad_packet(self):
        self.client.stop()

        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect(("localhost" ,SERVER_PORT))

        time.sleep(1)

        self.assertEqual(client_socket.recv(256), b'\x00\x00\x00*\x00\x00\x00\t\x00\x00\x00"Welcome! I am simple-mail-server.\x00')

        packet = struct.pack("!II", 4, 555)
        client_socket.send(packet)
        time.sleep(1)
        result = client_socket.recv(256)

        self.assertEqual(result, b'\x00\x00\x00\x08\x00\x00\x00\x08\x00\x00\x00\x03')

        packet = struct.pack("!II", 4, 10989)
        client_socket.send(packet)

        time.sleep(1)
        result = client_socket.recv(256)

        self.assertEqual(result, b'\x00\x00\x00\x08\x00\x00\x00\x08\x00\x00\x00\x03')

    # --- PRIVATE --- #

    def _connect(self, user=None):
        if self.client.is_running():
            self.client.stop()

        self.client.start()

        res = self.client.recv()

        assert res == WELCOME_MSG, "%s" % res

        if user is not None:
            self.client.send("User: %s\n" % user[0])
            self.client.send("Password: %s\n" % user[1])

            res = self._recv()

            assert res == CONNECTION_SUCCESSFUL_STR, "%s" % res


    def _send(self, cmd):
        return self.client.send(cmd)

    def _recv(self):
        return self.client.recv()


if __name__ == "__main__":
    unittest.main()
