import os
import random
import subprocess
from threading  import Thread
from queue import Queue

from pytests.commands import QUIT_CMD


CWD = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))

MAIL_SERVER_EXE = "./mail_server"
USERS_FILE = os.path.join(CWD, "pytests", "test_users_file")
EMPTY_USERS_FILE = os.path.join(CWD, "pytests", "empty_users_file")
SERVER_PORT = 5650 + random.randint(1, 19)

MAIL_CLIENT_EXE = "./mail_client"
DEFAULT_HOST = "localhost"


def enqueue_output(out, queue):
    while True:
        for line in iter(out.readline, b''):
            queue.put(line)


class Server(object):
    def __init__(self, exe=MAIL_SERVER_EXE, users_file=USERS_FILE, port=SERVER_PORT, cwd=CWD):
        self.exe = exe
        self.users_file = users_file
        self.port = port
        self.cwd = cwd

        self.mail_server = None

    def start(self):
        self.mail_server = subprocess.Popen([self.exe, self.users_file, str(self.port)],
                                            stdin=subprocess.PIPE,
                                            stdout=subprocess.PIPE,
                                            stderr=subprocess.PIPE,
                                            cwd=self.cwd)

    def stop(self):
        if self.mail_server is None:
            print("Server can't be terminated. Server is down.")
            return

        self.mail_server.terminate()


class Client(object):
    def __init__(self, exe=MAIL_CLIENT_EXE, host=DEFAULT_HOST, port=SERVER_PORT, cwd=CWD):
        self.exe = exe
        self.host = host
        self.port = port
        self.cwd = cwd

        self.mail_client = None
        self.running = False

    def start(self):
        self.mail_client = subprocess.Popen([self.exe, self.host, str(self.port)],
                                            stdin=subprocess.PIPE,
                                            stdout=subprocess.PIPE,
                                            stderr=subprocess.PIPE,
                                            cwd=self.cwd)
        self.pid = self.mail_client.pid
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
