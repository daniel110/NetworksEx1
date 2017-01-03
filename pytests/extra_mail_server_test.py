import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.realpath(__file__))))

import subprocess
import unittest

from pytests.common import MAIL_SERVER_EXE, EMPTY_USERS_FILE
from pytests.common import Client
from pytests.common import Server


class TestMailServer(unittest.TestCase):

    def test_running_server_with_empty_file(self):
        server = Server(users_file=EMPTY_USERS_FILE)
        server.start()

        pid = server.mail_server.pid

        p1 = subprocess.Popen(['ps', '-fade'], stdout=subprocess.PIPE)
        p2 = subprocess.Popen(['grep', str(pid)], stdin=p1.stdout, stdout=subprocess.PIPE)
        res = p2.communicate()[0].split(b'\n')

        self.assertGreater(res[0].decode().index(MAIL_SERVER_EXE), 0)

        # Starting client, try to login
        client = Client()
        client.start()
        client.recv()

        client.send("User: Clark\n")
        client.send("Password: Kent\n")

        res = client.recv()

        self.assertEqual(res, b"Failed on Login: Unknown user name.\n")


        # Make sure the server still running

        p1 = subprocess.Popen(['ps', '-fade'], stdout=subprocess.PIPE)
        p2 = subprocess.Popen(['grep', str(pid)], stdin=p1.stdout, stdout=subprocess.PIPE)
        res = p2.communicate()[0].split(b'\n')

        self.assertGreater(res[0].decode().index(MAIL_SERVER_EXE), 0)

        client.stop()
        server.stop()


if __name__ == "__main__":
    unittest.main()
