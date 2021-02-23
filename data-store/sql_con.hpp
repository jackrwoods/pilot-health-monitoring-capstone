#include <sqlite3.h>
#include <iostream>
#include <string>
#include <vector>

#include "ds_types.hpp"

// biometric samples table
#define SQL_SAMPLE_TABLE "samples"
#define SQL_SAMPLE_SCHEMA "( \
						sample_time BIGINT, \
						optical INT, \
						po2 INT \
					);"
#define SQL_SAMPLE_COLUMN_LIST "(sample_time, optical, po2)"

// pilot state table
#define SQL_STATE_TABLE "pilot_state"
#define SQL_STATE_SCHEMA "( \
						timestamp DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, \
						state INT \
					);"
#define SQL_STATE_COLUMN_LIST "(state)"

/**
 * SQL_Connection
 * Essentially a wrapper for a MYSQL object that implements necessary INSERT statements and table operations.
 */
class SQL_Connection
{
private:
	// MYSQL sql;
	sqlite3 *database;

	int query_execute(const char *c);

public:
	SQL_Connection();
	~SQL_Connection();

	int insert_samples(const std::vector<Sample> &v);
	int insert_sample(const Sample s);

	int insert_state(const Pilot_State s);
	int insert_states(const std::vector<Pilot_State> &v);
};

/**
 * query_execute: Execute a given SQL query
 * @param c SQL query to be executed
 * @returns zero on success, nonzero on error
 */
int SQL_Connection::query_execute(const char *c)
{
	char *error;
	int exit = sqlite3_exec(database, c, NULL, 0, &error);

	if (exit != SQLITE_OK)
	{
		std::cerr << "SQL Error: " << error << std::endl;
		sqlite3_free(error);
	}
	return 0;
}

SQL_Connection::SQL_Connection()
{
	// create sql connection - halt if error
	if (sqlite3_open("samples.db", &database))
	{
		std::cerr << "An error occurred connecting to the database" << sqlite3_errmsg(database) << std::endl;
		exit(1);
	}

	// Drop sample table if exists - halt if error
	const char *cmd_drop_sample_table = "DROP TABLE IF EXISTS " SQL_SAMPLE_TABLE ";";
	if (query_execute(cmd_drop_sample_table))
		exit(1);

	// Create new table for storing samples - halt if error
	const char *cmd_create_sample_table = "CREATE TABLE " SQL_SAMPLE_TABLE " " SQL_SAMPLE_SCHEMA;
	if (query_execute(cmd_create_sample_table))
		exit(1);

	// Drop state table if exists - halt if error
	const char *cmd_drop_state_table = "DROP TABLE IF EXISTS " SQL_STATE_TABLE ";";
	if (query_execute(cmd_drop_state_table))
		exit(1);

	// Create new table for storing pilot state - halt if error
	const char *cmd_create_state_table = "CREATE TABLE " SQL_STATE_TABLE " " SQL_STATE_SCHEMA;
	if (query_execute(cmd_create_state_table))
		exit(1);
}

SQL_Connection::~SQL_Connection()
{
	sqlite3_close(database);
}

/**
 * insert_samples: insert several po2/optical samples into the database
 * @param s vector of Sample structs
 * @returns zero on success, nonzero on error
 */
int SQL_Connection::insert_samples(const std::vector<Sample> &v)
{
	// construct sql statement
	// loop through each Sample in v and insert into statement
	std::string cmd_insert = "INSERT INTO " SQL_SAMPLE_TABLE " " SQL_SAMPLE_COLUMN_LIST " VALUES ";
	for (auto vi = v.begin(); vi != v.end(); vi++)
	{
		cmd_insert += "(" + std::to_string(vi->time_stamp.time_since_epoch().count()) + ',' + std::to_string(vi->optical) + ',' + std::to_string(vi->po2) + ")";

		if (vi + 1 != v.end())
			cmd_insert += ',';
	}
	cmd_insert += ';';

	return query_execute(cmd_insert.c_str());
}

/**
 * insert_sample: insert a single po2/optical sample into the database
 * @param s One Sample struct
 * @returns zero on success, nonzero on error
 */
int SQL_Connection::insert_sample(const Sample s)
{
	std::string cmd_insert = "INSERT INTO " SQL_SAMPLE_TABLE " " SQL_SAMPLE_COLUMN_LIST " VALUES (" + std::to_string(s.time_stamp.time_since_epoch().count()) + ',' + std::to_string(s.po2) + ',' + std::to_string(s.optical) + ");";
	return query_execute(cmd_insert.c_str());
}

/**
 * insert_state: insert a pilot state into database
 * @param s Current pilot state
 * @returns zero on success, nonzero on error
 */
int SQL_Connection::insert_state(const Pilot_State s)
{
	std::string cmd_insert = "INSERT INTO " SQL_STATE_TABLE " " SQL_STATE_COLUMN_LIST " VALUES(" + std::to_string(static_cast<int>(s)) + ");";
	return query_execute(cmd_insert.c_str());
}

/**
 * insert_states: Insert multiple pilot states into the database.
 * @param v std::vector of the last n recorded pilot states
 * @returns zero on success, nonzero on error
 */
int SQL_Connection::insert_states(const std::vector<Pilot_State> &v)
{
	std::string cmd_insert = "INSERT INTO " SQL_STATE_TABLE " " SQL_STATE_COLUMN_LIST " VALUES ";
	for (auto vi = v.begin(); vi != v.end(); vi++)
	{
		cmd_insert += "(" + std::to_string(static_cast<int>(*vi)) + ")";

		if (vi + 1 != v.end())
			cmd_insert += ',';
	}
	cmd_insert += ';';
	return query_execute(cmd_insert.c_str());
}