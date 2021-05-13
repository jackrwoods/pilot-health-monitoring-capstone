#ifndef SQL_DB
#define SQL_DB
#include <sqlite3.h>
#include <iostream>
#include <string>
#include <vector>

/**
 * SQL_Connection
 * Essentially a wrapper for a MYSQL object that implements necessary INSERT statements and table operations.
 */
class SQL_Connection
{
private:
	// Primary database object
	sqlite3* db;

	// Prepared statements (for fast execution time on frequently used queries)
	sqlite3_stmt* insertSample;
	sqlite3_stmt* selectAllSamples;

	int query_execute(const char *c);

public:
	SQL_Connection();
	~SQL_Connection();

	int insert_samples(const std::vector<Sample> &v);
	int insert_sample(Sample* s);
	int select_all_samples();
};

SQL_Connection::SQL_Connection() {
	// Opens a read/write connection to the sqlite database
	// Creates the database if one does not already exist
	if (sqlite3_open_v2(
		"./data/samples_database.db",
		&this->db,
		SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
		NULL // Empty string uses the default VFS module
	) != SQLITE_OK) std::cout << "Error creating database.\n";

	// If the samples table does not exist, create it
	this->query_execute("CREATE TABLE IF NOT EXISTS Samples(ID INTEGER PRIMARY KEY AUTOINCREMENT, Timestamp INTEGER NOT NULL, R_LED INTEGER, IR_LED INTEGER, Temperature REAL, BPM REAL, SpO2 REAL, PilotState INTEGER);");

	// Create prepared statements
	sqlite3_prepare_v2(
		this->db,
		"INSERT INTO Samples (ID, Timestamp, R_LED, IR_LED, Temperature, BPM, SpO2, PilotState) VALUES (NULL, ?, ?, ?, ?, ?, ?, ?)",
		65536, // Each of these queries can be a maximum of 64KB in length
		&this->insertSample,
		NULL
	);
	sqlite3_prepare_v2(
		this->db,
		"SELECT * FROM Samples;",
		65536, // Each of these queries can be a maximum of 64KB in length
		&this->selectAllSamples,
		NULL
	);
};

/**
 * query_execute: Execute a given SQL query
 * @param c SQL query to be executed
 * @returns zero on success, nonzero on error
 */
int SQL_Connection::query_execute(const char *c)
{
	// An errmsg pointer is required, and sqlite mallocs memory for a message each time.
	// We immediately free this after the query.
	char* errmsg;

	int result = sqlite3_exec(
		this->db,
		c,
		NULL,  /* Callback function */
		NULL, /* 1st argument to callback */
		&errmsg /* Error msg written here */
	);

	sqlite3_free(errmsg);

	return result;
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
	std::string cmd_insert = "INSERT INTO Samples(ID, Timestamp, R_LED, IR_LED, Temperature, BPM, SpO2, PilotState) VALUES ";
	for (auto vi = v.begin(); vi != v.end(); vi++)
	{
		cmd_insert += "(NULL, " + std::to_string(vi->timestamp) + ',' + std::to_string(vi->redLED) + ',' + std::to_string(vi->irLED) + ',' + std::to_string(0) + ',' + std::to_string(vi->bpm) + ',' + std::to_string(vi->spo2) + ',' + std::to_string(vi->pilot_state) + ")";


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
int SQL_Connection::insert_sample(Sample* s)
{
	// Bind the timestamp - long is a 32 bit integer, so 64 should be enough
	sqlite3_bind_int64(this->insertSample, 1, s->timestamp);

	// Bind everything else
	sqlite3_bind_double(this->insertSample, 2, s->redLED);
	sqlite3_bind_double(this->insertSample, 3, s->irLED);
	sqlite3_bind_double(this->insertSample, 4, 0); // this was temperature
	sqlite3_bind_double(this->insertSample, 5, s->bpm);
	sqlite3_bind_double(this->insertSample, 6, s->spo2);
	sqlite3_bind_int(this->insertSample, 7, s->pilot_state);

	// Execute the query
	int res = sqlite3_step(this->insertSample);

	// Reset the prepared statement
	sqlite3_reset(this->insertSample);
	return res;
}

/**
 * insert_sample: insert a single po2/optical sample into the database
 * @param s One Sample struct
 * @returns zero on success, nonzero on error
 */
int SQL_Connection::select_all_samples()
{
	// Execute the query
	int res = 0;
	while (sqlite3_step(this->selectAllSamples) == SQLITE_ROW) res++;

	// Reset the prepared statement
	sqlite3_reset(this->selectAllSamples);
	return res;
}

SQL_Connection::~SQL_Connection() {
	sqlite3_finalize(this->insertSample); // Frees memory associated with the prepared statement
	sqlite3_close(this->db);
}
#endif
