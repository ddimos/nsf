#include <iostream>

#include <nsf/NSF.hpp>
#include <cassert>
#include <unordered_set>

// #include <SFML/Graphics/RenderWindow.hpp>

constexpr nsf::Port SERVER_PORT = 20475;
constexpr nsf::Port CLIENT_PORT = 20480;

constexpr unsigned UPDATES_PER_SEC = 60;
constexpr float DT = 1.f / UPDATES_PER_SEC;

struct TestMessage
{
    sf::Uint64 field1 = 0;
    sf::Uint64 field2 = TestValue2;
    sf::Uint64 field3 = TestValue3;
    sf::Uint64 field4 = TestValue4;

    void serialize(nsf::Buffer& _buffer)
    {
        _buffer << field1;
        _buffer << field2;
        _buffer << field3;
        _buffer << field4;
    }
    void deserialize(nsf::Buffer& _buffer)
    {
        _buffer >> field1;
        _buffer >> field2;
        _buffer >> field3;
        _buffer >> field4;

        assert(field2 == TestValue2);
        assert(field3 == TestValue3);
        assert(field4 == TestValue4);
    }

    static sf::Uint64 TestValue1;
    constexpr static sf::Uint64 TestValue2 = 6548555;
    constexpr static sf::Uint64 TestValue3 = 1450;
    constexpr static sf::Uint64 TestValue4 = 4750;
};
sf::Uint64 TestMessage::TestValue1 = 0;

std::unordered_set<nsf::PeerID> g_peers;
bool m_tryToConnect = true;

int main(int argc, char **argv)
{
    bool isHost = true;
    if (argc < 2)
    {
        std::cout << "No arguments passed, assume to be a host" << std::endl;
    }
    else
    {
        std::cout << "Create a client" << std::endl;
        isHost = false;
    }

    nsf::Config config;
    config.port = (isHost) ? SERVER_PORT : CLIENT_PORT;
    config.isServer = isHost;
    nsf::NSFCallbacks callbacks;
    callbacks.onConnected = [](nsf::PeerID _peerId){
        std::cout << "--->>>CONNECTED<<<--- " << _peerId << std::endl;
        g_peers.insert(_peerId);
    };
    callbacks.onDisconnected = [](nsf::PeerID _peerId){
        std::cout << "--->>>DISCONNECTED<<<--- " << _peerId << std::endl;
        g_peers.erase(_peerId);
        m_tryToConnect = true;
    };
    callbacks.onReceived = [](nsf::NetworkMessage&& _message){
        TestMessage testMessage;
        testMessage.deserialize(_message.m_data);
        std::cout << ">Received from " << _message.getPeerId() << " data: " << testMessage.field1 << std::endl;
    };

    auto network = nsf::createNSF(config, callbacks);
    std::cout << "Network is created!" << std::endl;
    std::cout << "The public address : " << network->getPublicAddress().toString() << std::endl;
    std::cout << "The local address : "  << network->getLocalAddress().toString() << std::endl;


    // sf::RenderWindow m_window;
    // m_window.create(sf::VideoMode(1024, 768), "Planet");
	// m_window.setFramerateLimit(UPDATES_PER_SEC);

    bool isRunning = true;
    sf::Clock clock;
    float accumulator = 0.f;

    float timeUntilNextMessageS = 5.f;
    float timeUntilDisconnectS = 8.f;
    while (isRunning)
    {
        sf::Time elapsed = clock.restart();
        accumulator += elapsed.asSeconds();

//		m_game.update();
        while (accumulator >= DT)
        {
            network->updateReceive();

//		    m_game.fixedUpdate(DT);
            accumulator -= DT;

            network->updateSend();
        }

        if (!isHost && m_tryToConnect)
        {
            network->connect(nsf::NetworkAddress(sf::IpAddress("192.168.1.11"), SERVER_PORT));
            m_tryToConnect = false;
        }
        
        if (!g_peers.empty())
        {
            timeUntilNextMessageS -= elapsed.asSeconds();
            timeUntilDisconnectS -= elapsed.asSeconds();
        }

        if (timeUntilNextMessageS < 0)
        {
            timeUntilNextMessageS = !isHost ? 2:3;

            nsf::NetworkMessage message;
            message.m_info = nsf::MessageInfo(true);
            TestMessage testMessage;
            testMessage.field1 = TestMessage::TestValue1++;
            testMessage.serialize(message.m_data);

            std::cout << ">Send, data: " << testMessage.field1 << std::endl;
            network->send(std::move(message));
        }

        if (timeUntilDisconnectS < 0)
        {
            network->disconnect();
            timeUntilDisconnectS = 13;
        }

//      m_window.clear(sf::Color(2, 17, 34));
//		m_game.render();
//      m_window.display();
    }
}
