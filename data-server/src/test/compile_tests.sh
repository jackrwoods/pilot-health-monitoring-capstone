#!/bin/bash

# compile Data_Store test code

# ds_test.cpp - Runs tests for the Looping_Buffer object and multithreaded tests for the Data_Store object
g++ ds_test.cpp -lsqlite3 -lpthread -o ds_test.out

# sql.cpp - Runs example table creation and data insert routines
g++ sql.cpp -lsqlite3 -o sql_test.out

echo "Data_Store and Looping_Buffer tests compiled to ds_test.out (./ds_test.out)"
echo "SQL_Connection tests compiled to sql_test.out (./sql_test.out)"