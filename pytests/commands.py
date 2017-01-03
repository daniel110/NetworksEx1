WELCOME_MSG = b"Welcome! I am simple-mail-server.\n"
CONNECTION_SUCCESSFUL_STR = b"Connected to server\n"

SHOW_INBOX_CMD = "SHOW_INBOX\n"
GET_MAIL_CMD = "GET_MAIL %d\n"
DELETE_MAIL_CMD = "DELETE_MAIL %d\n"
QUIT_CMD = "QUIT\n"

COMPOSE_CMD = "COMPOSE\n"
COMPOSE_REPONSE = b"Mail sent\n"

RECV_MAIL_FORMAT = "From: %s\nTo: %s\nSubject: %s\nText: %s\n"
