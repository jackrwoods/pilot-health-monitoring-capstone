#include "../include/bluetooth_sensor_data_recv.hpp"
#include "../include/sql_con.hpp"

class Classifier {
    public:
        Classifier (BluetoothReceiver* bluetooth, SQL_Connection* db) {
            // Write a loop that
            //  1. Queries the db for samples (sample type is defined in include/datasource.hpp) (you can use db.query_execute that's defined in include/sql_con.hpp)
            //  2. evaluate your model and determine a classification
            //  3. call bluetooth->send_pilot_state() with a 1 (stressed) or a 0 (unstressed). 2 denotes that the pilot has been stressed for over a minute
        }
}