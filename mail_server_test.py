import socket
import subprocess
import unittest


BUFFER_SIZE = 1024

MAIL_SERVER_EXE = "mail_server"
USERS_FILE = "test_users_file"
SERVER_PORT = 6423
CONNECTION_SUCCESSFUL_STR = "Connected to server"
SHOW_INBOX_CMD = "SHOW_INBOX"
GET_MAIL_CMD = "GET_MAIL %d"
DELETE_MAIL_CMD = "DELETE_MAIL %d"
QUIT_CMD = "QUIT"
COMPOSE_CMD = "COMPOSE"
COMPOSE_REPONSE = "Mail sent"
RECV_MAIL_FORMAT = "From: %s\nTo: %s\nSubject: %s\nText: %s"


class TestMailServer(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.users = []
        with open(USERS_FILE, 'r') as fd:
            line = fd.readline().split('\t')
            cls.users.append((line[0], line[1]))

        cls.mail_server = subprocess.Popen([MAIL_SERVER_EXE, USERS_FILE, str(SERVER_PORT)],
                                            stdin=subprocess.PIPE,
                                            stdout=subprocess.PIPE,
                                            stderr=subprocess.PIPE)
        cls.client = None

    @classmethod
    def tearDownClass(cls):
        cls.mail_server.terminate()

    def tearDown(self):
        self._disconnect()

    def _get_user(self, name=None, index=0):
        if name is None:
            return self.users[index]

        for user in self.users:
            if name == user[0]:
                return user

        return None

    def test_connect(self):
        user = self._get_user(0)

        self._send("User: %s" % user[0])
        self._send("Password: %s" % user[1])

        self.assertEqual(self._recv(), CONNECTION_SUCCESSFUL_STR)

        self._disconnect()

    def test_compose(self):
        self._connect(self._get_user(0))

        user1 = self._get_user(1)
        to = "To: %s" % user1[0]
        subject = "Subject: Funny pictures"
        text = "Text: How are you? Long time no see!"

        self._send(COMPOSE_CMD)

        self._send(to)
        self._send(subject)
        self._send(text)

        self.assertEqual(self._recv(), COMPOSE_REPONSE)

        to = "To: %s" % user1[0]
        subject = "Subject: Avengers Internship"
        text = "Text: Please send your CV to the new exiting Avengers Internship! #TonyStark"

        self._send(COMPOSE_CMD)

        self._send(to)
        self._send(subject)
        self._send(text)

        self.assertEqual(self._recv(), COMPOSE_REPONSE)

    def test_show_inbox(self):
        expected_mail1 = '1 %s "Funny pictures"' % self._get_user(0)[0]
        expected_mail2 = '2 %s "Avengers Internship"' % self._get_user(0)[0]

        self._connect(self._get_user(1))

        self._send(SHOW_INBOX_CMD)

        self.assertEqual(self._recv(), "%s\n%s" % (expected_mail1, expected_mail2))

    def test_get_mail(self):
        user0 = self._get_user(0)
        user1 = self._get_user(1)

        frm = user0[0]
        to = user1[0]
        subject = "Funny pictures"
        text = "How are you? Long time no see!"

        self._connect(user1)

        self._send(GET_MAIL_CMD % 1)

        self.assertEqual(self._recv(), RECV_MAIL_FORMAT % (frm, to, subject, text))

    def test_delete_mail(self):
        expected_mail2 = '2 %s "Avengers Internship"' % self._get_user(0)[0]

        self._connect(self._get_user(1))

        self._send(DELETE_MAIL_CMD % 1)

        self._send(SHOW_INBOX_CMD)

        self.assertEqual(self._recv(), expected_mail2)

    def test_quit(self):
        # TODO: How to test quit?
        pass

    def _connect(self, user=None):
        if self.client is not None:
            self._disconnect()

        self.client = socket.socket()
        self.client.connect((socket.gethostname(), SERVER_PORT))

        if user is not None:
            self._send("User: %s" % user[0])
            self._send("Password: %s" % user[1])

            assert self._recv() == CONNECTION_SUCCESSFUL_STR


    def _disconnect(self):
        if self.client is not None:
            self._send(QUIT_CMD)
            self.client.close()
            self.client = None

    def _send(self, cmd):
        self.client.send(cmd.encode('utf-8'))

    def _recv(self):
        return self.client.recv(BUFFER_SIZE).decode('utf-8')



if __name__ == "__main__":
    unittest.main()
