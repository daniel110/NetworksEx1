/* server implementation */
#include "Server.h"
#include <errno.h>
#include <fcntl.h>          // open
#include <unistd.h>         // close / read / write / lseek
#include <stdio.h>


bool server::loadUsersFromFile(char * filePatch)
{
	std::list<User*> * user_list = getUsersFromFile(filePatch);
	if (user_list == nullptr)
	{
		return false;
	}

	createInboxList(*user_list);

	delete user_list;

	clearSessionList();

	return true;
}

User * server::getUserFromLine(char * buf, int size)
{
	std::string user;
	std::string pass;
	char * ptr_start = buf;
	char * ptr_cur = buf;

	if (buf == NULL)
	{
		return nullptr;
	}

	while ((*ptr_cur != '\t') && ((ptr_cur - ptr_start) < size))
	{
		ptr_cur++;
	}

	if ((ptr_cur - ptr_start) == 0)
	{
		return nullptr;
	}

	user.assign(ptr_start, ptr_cur - ptr_start);
	size -= (ptr_cur - ptr_start);
	ptr_start = ptr_cur;

	while ((*ptr_cur != '\n') && ((ptr_cur - ptr_start) < size))
	{
		ptr_cur++;
	}

	if ((ptr_cur - ptr_start) == 0)
	{
		return nullptr;
	}

	pass.assign(ptr_start, ptr_cur - ptr_start);

	return new User(user, pass);
}

std::list<User*> * server::getUsersFromFile(char * filePatch)
{
	FILE * hdl_input = 0;
	char * buf = NULL;
	size_t buf_len = 0;
	ssize_t read_count = 0;
	std::list<User*> * user_list = new std::list<User*>();

    /*  Get key file descriptor */
    hdl_input = fopen(filePatch, "r");
    if (hdl_input < 0)
    {
		printf("Error opening user files: %s\n%s\n", strerror(errno), filePatch);
		return nullptr;
    }

    /* getline gets a char** and sets a newly allocated buffer.
     * If the pointer contains not NULL, it reallocs if necessary */
    read_count = getline(&buf, &buf_len, hdl_input);
    while (read_count > 0)
    {

    	User * nuser = getUserFromLine(buf, buf_len);
    	if (nuser == NULL)
    	{
    		break;
    	}

    	user_list->push_back(nuser);

    	read_count = getline(&buf, &buf_len, hdl_input);
    }


    free(buf);
    fclose(hdl_input);

    return user_list;
}

void server::clearInboxList()
{
	Inbox * cur_inbox = nullptr;

	while (m_all_inbox.empty() == false)
	{
		cur_inbox = m_all_inbox.back();

		delete cur_inbox;

		m_all_inbox.pop_back();
	}

}

void server::clearSessionList()
{
	ServerSessionSocket * cur_session = nullptr;

	while (m_sessions.empty() == false)
	{
		cur_session = m_sessions.back();

		cur_session->close();

		delete cur_session;

		m_sessions.pop_back();
	}
}

void server::createInboxList(std::list<User*>& users)
{
	User * cur_user = nullptr;
	Inbox * nInbox = nullptr;

	clearInboxList();

	while (users.empty() == false)
	{
		cur_user = users.back();
		nInbox = new Inbox(*cur_user);
		m_all_inbox.push_back(nInbox);

		delete cur_user;

		users.pop_back();
	}


}
