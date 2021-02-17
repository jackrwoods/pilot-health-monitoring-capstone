#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <random>
#include <vector>

// min/max for number of po2 samples to include in each packet
#define MAX_TRANSMIT_SIZE 64
#define MIN_TRANSMIT_SIZE 32

// https://people.csail.mit.edu/albert/bluez-intro/x559.html
// uses l2cap sockets

namespace Bluetooth_Connection
{

    struct Connection
    {
        char address[19]{0};
        char name[248]{0};
    };

    class Bluetooth_Connection
    {
    private:
        int average{0};
        int variance{0};

        uint32_t data_buffer[MAX_TRANSMIT_SIZE];

    public:
        Bluetooth_Connection(std::string addr);
        ~Bluetooth_Connection();

        bool open_connection(std::string addr);
        bool close_connection();

        int adjust_po2(int adj);

        std::vector<Connection> available_connections();

        bool active();
    };

    Bluetooth_Connection::Bluetooth_Connection(std::string addr)
    {
        open_connection(addr);
    }

    Bluetooth_Connection::~Bluetooth_Connection()
    {
    }

    /**
 * open_connection: Creates a bluetooth connection.
 * @param addr the bluetooth address of device to connect to.
 * @returns True if a connection was created, false otherwise.
 */
    bool Bluetooth_Connection::open_connection(std::string addr)
    {
        return false;
    }

    /**
 * close_connection: Closes any open bluetooth connections.
 * @returns True if a connection was closed, false otherwise.
 */
    bool Bluetooth_Connection::close_connection()
    {
        return false;
    }

    /**
 * adjust_po2: Adjust the average PO2 level sent by the object.
 * @param adj The amount to adjust the PO2 level by.
 * @returns The amount that the PO2 was adjusted by.
 */
    int Bluetooth_Connection::adjust_po2(int adj)
    {
        return 0;
    }

    /**
 * active: Is there an active bluetooth connection.
 * @returns True if there is an active bluetooth connection, false otherwise.
 */
    bool Bluetooth_Connection::active()
    {
        return false;
    }

    /**
 * available_connections: Construct and return a vector of available bluetooth connections.
 * @returns std::vector<Connection> listing all available bluetooth connections.
 */
    std::vector<Connection> available_connections()
    {
        // most of this code was adapted(stolen) from: https://people.csail.mit.edu/albert/bluez-intro/c404.html


        std::vector<Connection> cons;

        inquiry_info *ii {nullptr};
        int max_rsp, num_rsp;
        int dev_id, sock, len, flags;
        int i;

        dev_id = hci_get_route(nullptr);
        sock = hci_open_dev(dev_id);
        if (dev_id < 0 || sock < 0)
        {
            std::cerr << "An error occurred opening the bluetooth socket." << std::endl;
            return cons;
        }

        len = 8;
        max_rsp = 255;
        flags = IREQ_CACHE_FLUSH;
        ii = (inquiry_info *)malloc(max_rsp * sizeof(inquiry_info));

        num_rsp = hci_inquiry(dev_id, len, max_rsp, nullptr, &ii, flags);
        if (num_rsp < 0)
            std::cerr << "The hci_inquiry function encountered an error." << std::endl;

        for (i = 0; i < num_rsp; i++)
        {
            cons.push_back(Connection());
            ba2str(&(ii + i)->bdaddr, cons.back().address);
            memset(cons.back().name, 0, sizeof(cons.back().name));
            if (hci_read_remote_name(sock, &(ii + i)->bdaddr, sizeof(cons.back().name), cons.back().name, 0) < 0)
                strcpy(cons.back().name, "[unknown]");
        }

        free(ii);
        close(sock);

        return cons;
    }

} // namespace Bluetooth_Connection