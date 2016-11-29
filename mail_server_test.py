import random
import subprocess
from threading  import Thread
from queue import Queue, Empty
import unittest


MAIL_SERVER_EXE = "./mail_server"
USERS_FILE = "test_users_file"
SERVER_PORT = 5650 + random.randint(1, 19)

MAIL_CLIENT_EXE = "./mail_client"
DEFAULT_HOST = "localhost"

WELCOME_MSG = b"Welcome! I am simple-mail-server.\n"
CONNECTION_SUCCESSFUL_STR = b"Connected to server\n"

SHOW_INBOX_CMD = "SHOW_INBOX\n"
GET_MAIL_CMD = "GET_MAIL %d\n"
DELETE_MAIL_CMD = "DELETE_MAIL %d\n"
QUIT_CMD = "QUIT\n"

COMPOSE_CMD = "COMPOSE\n"
COMPOSE_REPONSE = b"Mail sent\n"

RECV_MAIL_FORMAT = "From: %s\nTo: %s\nSubject: %s\nText: %s\n"


def enqueue_output(out, queue):
    while True:
        for line in iter(out.readline, b''):
            queue.put(line)


class Server(object):
    def __init__(self, exe=MAIL_SERVER_EXE, users_file=USERS_FILE, port=SERVER_PORT):
        self.exe = exe
        self.users_file = users_file
        self.port = port

        self.mail_server = None

    def start(self):
        self.mail_server = subprocess.Popen([self.exe, self.users_file, str(self.port)],
                                            stdin=subprocess.PIPE,
                                            stdout=subprocess.PIPE,
                                            stderr=subprocess.PIPE)

    def stop(self):
        if self.mail_server is None:
            print("Server can't be terminated. Server is down.")
            return

        self.mail_server.terminate()


class Client(object):
    def __init__(self, exe=MAIL_CLIENT_EXE, host=DEFAULT_HOST, port=SERVER_PORT):
        self.exe = exe
        self.host = host
        self.port = port

        self.mail_client = None
        self.running = False

    def start(self):
        self.mail_client = subprocess.Popen([self.exe, self.host, str(self.port)],
                                            stdin=subprocess.PIPE,
                                            stdout=subprocess.PIPE,
                                            stderr=subprocess.PIPE)
        self.q = Queue()
        self.t = Thread(target=enqueue_output, args=(self.mail_client.stdout, self.q))
        self.t.daemon = True  # thread dies with the program
        self.t.start()

        self.running = True

    def stop(self):
        if self.mail_client is None:
            print("Client can't be terminated. Client is down.")
            return

        self.send(QUIT_CMD)
        self.mail_client.terminate()
        self.running = False

    def is_running(self):
        return self.running

    def send(self, data):
        """Sends the given data to the server"""
        result = self.mail_client.stdin.write(data.encode('utf-8'))
        if data == QUIT_CMD:
            return

        self.mail_client.stdin.flush()
        return result

    def recv(self):
        return self.q.get(timeout=2)


class TestMailServer(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.users = []
        with open(USERS_FILE, 'r') as fd:
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

        # self._connect(user)

        if not self.client.is_running():
            self.client.start()
            self._recv()

        self._send("User: %s\n" % user[0])
        self._send("Password: %s\n" % user[1])

        self.assertEqual(self._recv(), CONNECTION_SUCCESSFUL_STR)

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

    def test_e_delete_mail(self):
        expected_mail2 = '2 %s "Avengers Internship"\n' % self._get_user(0)[0]

        self._connect(self._get_user(1))

        self._send(DELETE_MAIL_CMD % 1)

        self._send(SHOW_INBOX_CMD)

        self.assertEqual(self._recv(), expected_mail2.encode('utf-8'))

    def test_f_quit(self):
        # TODO: How to test quit?
        pass

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
