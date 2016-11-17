#pragma once

class PacketCreator
{
    /*  Server
    ************* */
    static Packet& getWelcomePacket();
    static Packet& getShowInboxResponseAsPacket(Inbox& box);
    static Packet& getGetMailResponseAsPacket(MailObj& mail);
    static Packet& getDeleteResponseAsPacket(int state);
    static Packet& getLogingStatePacket(User& user, int state);
    static Packet& getComposeResponseAsPacket(int state);
    static Packet& getStatusResponseAsPacket(int state);


    /*  Client
    ************* */
    static Packet& getShowInboxRequestAsPacket();
    static Packet& getGetMailRequestAsPacket(unsigned long id);
    static Packet& getDeleteRequestAsPacket(unsigned long id);
    static Packet& getLoginRequestAsPacket(unsigned long id);
    static Packet& getComposeRequestAsPacket(MailObj& mail);
};
