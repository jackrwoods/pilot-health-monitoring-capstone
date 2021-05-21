require('dotenv').config()
const express = require('express')
const app = express()
const port = process.env.PORT != null ? process.env.PORT : 3000

app.listen(port, () => {
    console.log(`Webserver started on port ${port}`)
    console.log(`Use ctrl + c to exit.`)
})

// API Routes
const json2csv = require('json2csv')
const parser = new json2csv.Parser()
const sqlite3 = require('sqlite3').verbose()
const dbLocation = process.env.DB != null ? process.env.DB : '../data-server/data/samples_database.db'
let db = new sqlite3.Database(dbLocation, sqlite3.OPEN_READONLY, (error) => {
	console.log('Connecting to ' + dbLocation)
	if (error) console.log(error)
	else console.log('Connected to DB.')
})
app.get('/api/csv', (request, result) => {
	db.run('.mode csv', (error, rows) => {
        if (error) {
			console.log(error)
		} else {
			const filename = 'samples-' + new Date().getTime() + '.csv';
			db.run('.output ' + filename, (error) => {
				if (error) {
					console.log(error)
				} else {
					db.run('SELECT * FROM Samples', (error) => {
						if (error) {
							console.log(error)
						} else {
							result.header('Content-Type', 'text/csv')
							return result.sendFile(filename)
						}
					})
				}
			})
		}
    })
    db.all('SELECT * FROM Samples', [], (error, rows) => {
        if (error) {
			console.log(error)
		} else {
			result.header('Content-Type', 'text/csv')
			result.attachment('samples-' + new Date().getTime() + '.csv')
			return result.send(parser.parse(rows))
		}
    })
})

// Host frontend static content
app.use(express.static('build'))
