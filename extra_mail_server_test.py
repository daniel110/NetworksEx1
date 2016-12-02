import random
import subprocess
import socket
import struct
from threading  import Thread
from queue import Queue, Empty
import unittest
import time
import os


MAIL_SERVER_EXE = "./mail_server"
USERS_FILE = "users_file"
EMPTY_USERS_FILE = "empty_users_file"
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


class TestMailServer(unittest.TestCase):

    def test_running_server_with_empty_file(self):
        server = Server(users_file=EMPTY_USERS_FILE)
        server.start()

        pid = server.mail_server.pid

        p1 = subprocess.Popen(['ps', '-fade'], stdout=subprocess.PIPE)
        p2 = subprocess.Popen(['grep', str(pid)], stdin=p1.stdout, stdout=subprocess.PIPE)
        res = p2.communicate()[0].split(b'\n')

        self.assertGreater(res[0].decode().index(MAIL_SERVER_EXE), 0)

        server.stop()

    def test_running_server_with_bad_port(self):
        server = Server(port=99999999999)
        server.start()

        pid = server.mail_server.pid

        p1 = subprocess.Popen(['ps', '-fade'], stdout=subprocess.PIPE)
        p2 = subprocess.Popen(['grep', str(pid)], stdin=p1.stdout, stdout=subprocess.PIPE)
        res = p2.communicate()[0].split(b'\n')

        self.assertGreater(res[0].decode().index(MAIL_SERVER_EXE), 0)

        server.stop()


if __name__ == "__main__":
    unittest.main()
